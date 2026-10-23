#pragma once
#include "Common.h"

class StereoPipeline {
public:
    StereoPipeline() = default;
    ~StereoPipeline() { Reset(); }

    bool EnsureResources(ID3D11Device* armaDevice, ID3D11Device* vrDevice, uint32_t width, uint32_t height);
    void Reset();

    void ProcessFrame(
        ID3D11DeviceContext* armaContext,
        ID3D11Texture2D* sourceColor,
        ID3D11Texture2D* sourceDepth,
        bool stereoEnabled,
        float separation,
        float convergence
    );

    ID3D11Texture2D* GetLeftEyeVRTexture() const { return m_vrSharedTextureL.Get(); }
    ID3D11Texture2D* GetRightEyeVRTexture() const { return m_vrSharedTextureR.Get(); }

private:
    struct StereoParams {
        float separation;
        float convergence;
        float eyeSign; // -1.0 = Left eye, +1.0 = Right eye
        float padding;
    };

    uint32_t m_width = 0;
    uint32_t m_height = 0;

    // Shader resources
    ComPtr<ID3D11VertexShader> m_stereoVS;
    ComPtr<ID3D11PixelShader> m_stereoPS;
    ComPtr<ID3D11SamplerState> m_sampler;
    ComPtr<ID3D11Buffer> m_constantBuffer;

    // Intermediate shader-readable copies
    ComPtr<ID3D11Texture2D> m_colorCopy;
    ComPtr<ID3D11ShaderResourceView> m_colorSRV;
    ComPtr<ID3D11Texture2D> m_depthReadable;
    ComPtr<ID3D11ShaderResourceView> m_depthSRV;

    // DXGI Shared surfaces between Arma device & OpenXR VR device
    ComPtr<ID3D11Texture2D> m_armaSharedTextureL;
    ComPtr<ID3D11Texture2D> m_vrSharedTextureL;
    ComPtr<ID3D11RenderTargetView> m_armaRTVL;

    ComPtr<ID3D11Texture2D> m_armaSharedTextureR;
    ComPtr<ID3D11Texture2D> m_vrSharedTextureR;
    ComPtr<ID3D11RenderTargetView> m_armaRTVR;
};