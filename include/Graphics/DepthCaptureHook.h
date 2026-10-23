#pragma once
#include "Common.h"

class DepthCaptureHook {
public:
    static DepthCaptureHook& Instance();

    void Install(ID3D11DeviceContext* context);
    void Remove(ID3D11DeviceContext* context);

    // Returns an AddRef'd pointer to the captured depth texture, or nullptr if none
    ComPtr<ID3D11Texture2D> GetCapturedDepthTexture();

private:
    DepthCaptureHook() = default;

    using PFN_OMSetRenderTargets = void(__stdcall*)(
        ID3D11DeviceContext*, UINT, ID3D11RenderTargetView* const*, ID3D11DepthStencilView*
        );

    static void __stdcall Hook_OMSetRenderTargets(
        ID3D11DeviceContext* This,
        UINT NumViews,
        ID3D11RenderTargetView* const* ppRenderTargetViews,
        ID3D11DepthStencilView* pDepthStencilView
    );

    PFN_OMSetRenderTargets m_originalOMSetRenderTargets = nullptr;
    ComPtr<ID3D11DepthStencilView> m_capturedDSV;
    std::mutex m_mutex;
};