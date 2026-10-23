#include "Graphics/StereoPipeline.h"

namespace {
    const char* kStereoShaderHLSL = R"(
        Texture2D g_Color : register(t0);
        Texture2D g_Depth : register(t1);
        SamplerState g_Sampler : register(s0);

        cbuffer StereoBuffer : register(b0)
        {
            float g_Separation;
            float g_Convergence;
            float g_EyeSign;
            float g_Padding;
        };

        struct VS_OUTPUT
        {
            float4 pos : SV_POSITION;
            float2 uv  : TEXCOORD0;
        };

        VS_OUTPUT VSMain(uint id : SV_VertexID)
        {
            VS_OUTPUT output;
            output.uv = float2((id << 1) & 2, id & 2);
            output.pos = float4(output.uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
            return output;
        }

        float4 PSMain(VS_OUTPUT input) : SV_Target
        {
            float rawZ = g_Depth.Sample(g_Sampler, input.uv).r;
            float depth = pow(saturate(1.0f - rawZ), 1.35f);

            float maxShift = g_Separation * g_EyeSign;
            float disparity = (depth - g_Convergence) * maxShift;
            float2 shiftedUV = input.uv + float2(disparity, 0.0f);
            shiftedUV.x = clamp(shiftedUV.x, 0.001f, 0.999f);

            float targetRawZ = g_Depth.Sample(g_Sampler, shiftedUV).r;
            float fgBleed = rawZ - targetRawZ;
            if (fgBleed > 0.015f)
            {
                float cancelFactor = smoothstep(0.015f, 0.055f, fgBleed);
                shiftedUV = lerp(shiftedUV, input.uv, cancelFactor);
            }

            return g_Color.Sample(g_Sampler, shiftedUV);
        }
    )";

    struct D3D11StateScopeBackup {
        ID3D11DeviceContext* ctx = nullptr;
        D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
        ComPtr<ID3D11InputLayout> inputLayout;
        ComPtr<ID3D11VertexShader> vs;
        ComPtr<ID3D11PixelShader> ps;
        ComPtr<ID3D11SamplerState> sampler;
        ComPtr<ID3D11Buffer> psCb;
        ComPtr<ID3D11ShaderResourceView> psSRVs[2];
        ComPtr<ID3D11RenderTargetView> rtvs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
        ComPtr<ID3D11DepthStencilView> dsv;
        UINT numViewports = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
        D3D11_VIEWPORT viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE]{};

        explicit D3D11StateScopeBackup(ID3D11DeviceContext* context) : ctx(context) {
            ctx->IAGetPrimitiveTopology(&topology);
            ctx->IAGetInputLayout(&inputLayout);
            ctx->VSGetShader(&vs, nullptr, nullptr);
            ctx->PSGetShader(&ps, nullptr, nullptr);
            ctx->PSGetSamplers(0, 1, sampler.GetAddressOf());
            ctx->PSGetConstantBuffers(0, 1, psCb.GetAddressOf());

            ID3D11ShaderResourceView* rawSRVs[2] = { nullptr };
            ctx->PSGetShaderResources(0, 2, rawSRVs);
            psSRVs[0].Attach(rawSRVs[0]);
            psSRVs[1].Attach(rawSRVs[1]);

            ID3D11RenderTargetView* rawRTVs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { nullptr };
            ID3D11DepthStencilView* rawDSV = nullptr;
            ctx->OMGetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, rawRTVs, &rawDSV);
            for (int i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i) rtvs[i].Attach(rawRTVs[i]);
            dsv.Attach(rawDSV);

            ctx->RSGetViewports(&numViewports, viewports);
        }

        ~D3D11StateScopeBackup() {
            ctx->RSSetViewports(numViewports, viewports);

            ID3D11RenderTargetView* rawRTVs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
            for (int i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i) rawRTVs[i] = rtvs[i].Get();
            ctx->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, rawRTVs, dsv.Get());

            ctx->IASetPrimitiveTopology(topology);
            ctx->IASetInputLayout(inputLayout.Get());
            ctx->VSSetShader(vs.Get(), nullptr, 0);
            ctx->PSSetShader(ps.Get(), nullptr, 0);
            ctx->PSSetSamplers(0, 1, sampler.GetAddressOf());
            ctx->PSSetConstantBuffers(0, 1, psCb.GetAddressOf());

            ID3D11ShaderResourceView* rawSRVs[2] = { psSRVs[0].Get(), psSRVs[1].Get() };
            ctx->PSSetShaderResources(0, 2, rawSRVs);
        }
    };
}

void StereoPipeline::Reset() {
    m_stereoVS.Reset();
    m_stereoPS.Reset();
    m_sampler.Reset();
    m_constantBuffer.Reset();
    m_colorCopy.Reset();
    m_colorSRV.Reset();
    m_depthReadable.Reset();
    m_depthSRV.Reset();
    m_armaSharedTextureL.Reset();
    m_vrSharedTextureL.Reset();
    m_armaRTVL.Reset();
    m_armaSharedTextureR.Reset();
    m_vrSharedTextureR.Reset();
    m_armaRTVR.Reset();
    m_width = 0;
    m_height = 0;
}

bool StereoPipeline::EnsureResources(
    ID3D11Device* armaDevice,
    ID3D11Device* vrDevice,
    uint32_t width,
    uint32_t height)
{
    if (m_stereoPS && m_depthReadable && m_width == width && m_height == height) {
        return true;
    }

    Reset();

    // 1. Compile Shaders
    ComPtr<ID3DBlob> vsBlob, psBlob, errBlob;
    if (FAILED(D3DCompile(kStereoShaderHLSL, strlen(kStereoShaderHLSL), nullptr, nullptr, nullptr, "VSMain", "vs_4_0", 0, 0, &vsBlob, &errBlob)))
        return false;
    armaDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_stereoVS);

    if (FAILED(D3DCompile(kStereoShaderHLSL, strlen(kStereoShaderHLSL), nullptr, nullptr, nullptr, "PSMain", "ps_4_0", 0, 0, &psBlob, &errBlob)))
        return false;
    armaDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_stereoPS);

    D3D11_SAMPLER_DESC sampDesc{};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    armaDevice->CreateSamplerState(&sampDesc, &m_sampler);

    D3D11_BUFFER_DESC bDesc{};
    bDesc.ByteWidth = sizeof(StereoParams);
    bDesc.Usage = D3D11_USAGE_DEFAULT;
    bDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    armaDevice->CreateBuffer(&bDesc, nullptr, &m_constantBuffer);

    // 2. Allocate Color Copy Texture
    D3D11_TEXTURE2D_DESC colorDesc{};
    colorDesc.Width = width;
    colorDesc.Height = height;
    colorDesc.MipLevels = 1;
    colorDesc.ArraySize = 1;
    colorDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    colorDesc.SampleDesc.Count = 1;
    colorDesc.Usage = D3D11_USAGE_DEFAULT;
    colorDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    armaDevice->CreateTexture2D(&colorDesc, nullptr, &m_colorCopy);
    armaDevice->CreateShaderResourceView(m_colorCopy.Get(), nullptr, &m_colorSRV);

    // 3. Allocate Typeless Readable Depth Texture
    D3D11_TEXTURE2D_DESC depthDesc{};
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    armaDevice->CreateTexture2D(&depthDesc, nullptr, &m_depthReadable);

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    armaDevice->CreateShaderResourceView(m_depthReadable.Get(), &srvDesc, &m_depthSRV);

    // 4. Create Shared Surfaces for Left and Right Eyes
    auto CreateSharedEye = [&](ComPtr<ID3D11Texture2D>& armaTex, ComPtr<ID3D11Texture2D>& vrTex, ComPtr<ID3D11RenderTargetView>& rtv) {
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

        armaDevice->CreateTexture2D(&desc, nullptr, &armaTex);
        armaDevice->CreateRenderTargetView(armaTex.Get(), nullptr, &rtv);

        ComPtr<IDXGIResource> res;
        armaTex.As(&res);
        HANDLE sharedHandle = nullptr;
        res->GetSharedHandle(&sharedHandle);
        vrDevice->OpenSharedResource(sharedHandle, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(vrTex.GetAddressOf()));
        };

    CreateSharedEye(m_armaSharedTextureL, m_vrSharedTextureL, m_armaRTVL);
    CreateSharedEye(m_armaSharedTextureR, m_vrSharedTextureR, m_armaRTVR);

    m_width = width;
    m_height = height;
    return true;
}

void StereoPipeline::ProcessFrame(
    ID3D11DeviceContext* armaContext,
    ID3D11Texture2D* sourceColor,
    ID3D11Texture2D* sourceDepth,
    bool stereoEnabled,
    float separation,
    float convergence)
{
    if (!stereoEnabled || !sourceDepth) {
        armaContext->CopyResource(m_armaSharedTextureL.Get(), sourceColor);
        armaContext->CopyResource(m_armaSharedTextureR.Get(), sourceColor);
        armaContext->Flush();
        return;
    }

    // 1. Copy active buffers into readable textures
    armaContext->CopyResource(m_colorCopy.Get(), sourceColor);
    armaContext->CopyResource(m_depthReadable.Get(), sourceDepth);

    // 2. Backup context state with RAII
    {
        D3D11StateScopeBackup backup(armaContext);

        D3D11_VIEWPORT vp{ 0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height), 0.0f, 1.0f };
        armaContext->RSSetViewports(1, &vp);
        armaContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        armaContext->IASetInputLayout(nullptr);
        armaContext->VSSetShader(m_stereoVS.Get(), nullptr, 0);
        armaContext->PSSetShader(m_stereoPS.Get(), nullptr, 0);
        armaContext->PSSetSamplers(0, 1, m_sampler.GetAddressOf());

        ID3D11ShaderResourceView* views[2] = { m_colorSRV.Get(), m_depthSRV.Get() };
        armaContext->PSSetShaderResources(0, 2, views);

        // Render Left Eye (eyeSign = -1.0)
        StereoParams paramsL{ separation, convergence, -1.0f, 0.0f };
        armaContext->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &paramsL, 0, 0);
        armaContext->PSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
        armaContext->OMSetRenderTargets(1, m_armaRTVL.GetAddressOf(), nullptr);
        armaContext->Draw(3, 0);

        // Render Right Eye (eyeSign = +1.0)
        StereoParams paramsR{ separation, convergence, 1.0f, 0.0f };
        armaContext->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &paramsR, 0, 0);
        armaContext->PSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
        armaContext->OMSetRenderTargets(1, m_armaRTVR.GetAddressOf(), nullptr);
        armaContext->Draw(3, 0);

        ID3D11ShaderResourceView* nullViews[2] = { nullptr, nullptr };
        armaContext->PSSetShaderResources(0, 2, nullViews);
    }

    armaContext->Flush();
}