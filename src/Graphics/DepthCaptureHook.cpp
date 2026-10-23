#include "Graphics/DepthCaptureHook.h"

DepthCaptureHook& DepthCaptureHook::Instance() {
    static DepthCaptureHook s_instance;
    return s_instance;
}

void DepthCaptureHook::Install(ID3D11DeviceContext* context) {
    if (m_originalOMSetRenderTargets != nullptr || context == nullptr) {
        return;
    }

    void** vtbl = *reinterpret_cast<void***>(context);
    DWORD oldProtect = 0;
    if (VirtualProtect(&vtbl[33], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect)) {
        m_originalOMSetRenderTargets = reinterpret_cast<PFN_OMSetRenderTargets>(vtbl[33]);
        vtbl[33] = reinterpret_cast<void*>(&Hook_OMSetRenderTargets);
        VirtualProtect(&vtbl[33], sizeof(void*), oldProtect, &oldProtect);
    }
}

void DepthCaptureHook::Remove(ID3D11DeviceContext* context) {
    if (m_originalOMSetRenderTargets == nullptr || context == nullptr) {
        return;
    }

    void** vtbl = *reinterpret_cast<void***>(context);
    DWORD oldProtect = 0;
    if (VirtualProtect(&vtbl[33], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect)) {
        vtbl[33] = reinterpret_cast<void*>(m_originalOMSetRenderTargets);
        VirtualProtect(&vtbl[33], sizeof(void*), oldProtect, &oldProtect);
        m_originalOMSetRenderTargets = nullptr;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    m_capturedDSV.Reset();
}

void __stdcall DepthCaptureHook::Hook_OMSetRenderTargets(
    ID3D11DeviceContext* This,
    UINT NumViews,
    ID3D11RenderTargetView* const* ppRenderTargetViews,
    ID3D11DepthStencilView* pDepthStencilView)
{
    auto& hook = Instance();

    if (pDepthStencilView != nullptr) {
        ComPtr<ID3D11Resource> res;
        pDepthStencilView->GetResource(&res);

        ComPtr<ID3D11Texture2D> tex;
        if (res && SUCCEEDED(res.As(&tex))) {
            D3D11_TEXTURE2D_DESC desc{};
            tex->GetDesc(&desc);

            // Filter out small shadow maps (only capture main scene depth: >= 720p)
            if (desc.Width >= 1280 && desc.Height >= 720) {
                std::lock_guard<std::mutex> lock(hook.m_mutex);
                hook.m_capturedDSV = pDepthStencilView;
            }
        }
    }

    hook.m_originalOMSetRenderTargets(This, NumViews, ppRenderTargetViews, pDepthStencilView);
}

ComPtr<ID3D11Texture2D> DepthCaptureHook::GetCapturedDepthTexture() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_capturedDSV) {
        return nullptr;
    }

    ComPtr<ID3D11Resource> res;
    m_capturedDSV->GetResource(&res);

    ComPtr<ID3D11Texture2D> tex;
    if (res) {
        res.As(&tex);
    }
    return tex;
}