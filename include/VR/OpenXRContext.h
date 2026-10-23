#pragma once
#include "Common.h"
#include "Graphics/StereoPipeline.h"

class OpenXRContext {
public:
    struct SwapchainData {
        XrSwapchain handle = XR_NULL_HANDLE;
        int32_t width = 0;
        int32_t height = 0;
        std::vector<XrSwapchainImageD3D11KHR> images;
    };

    OpenXRContext() = default;
    ~OpenXRContext() { Shutdown(); }

    OpenXRContext(const OpenXRContext&) = delete;
    OpenXRContext& operator=(const OpenXRContext&) = delete;

    bool Initialize(std::string& outError);
    bool StartSession(ID3D11Device* device, ID3D11DeviceContext* context, std::string& outError);
    void Shutdown();
    void PollEvents();

    bool RenderFrame(
        ID3D11DeviceContext* context,
        ID3D11Texture2D* sourceTexture,
        ID3D11Texture2D* depthTexture,
        float& outYaw, float& outPitch, float& outRoll,
        std::string& diagInfo, std::string& outError
    );

    bool GetD3D11Requirements(XrGraphicsRequirementsD3D11KHR& requirements, std::string& outError);

    // Stereo Mode Controls
    void SetStereoMode(bool enabled) { m_stereoEnabled = enabled; }
    bool IsStereoMode() const { return m_stereoEnabled; }
    bool ToggleStereoMode() { m_stereoEnabled = !m_stereoEnabled; return m_stereoEnabled; }
    void AdjustSeparation(float delta) { m_separation = std::clamp(m_separation + delta, 0.000f, 0.030f); }
    void AdjustConvergence(float delta) { m_convergence = std::clamp(m_convergence + delta, -0.20f, 0.50f); }
    float GetSeparation() const { return m_separation; }
    float GetConvergence() const { return m_convergence; }

    bool IsInitialized() const { return m_initialized; }
    bool IsSessionCreated() const { return m_session != XR_NULL_HANDLE; }
    bool IsSessionRunning() const { return m_sessionRunning; }
    XrSessionState GetSessionState() const { return m_sessionState; }
    std::string GetStatus() const;

    static const char* ResultToString(XrResult result);

private:
    bool CreateInstance(std::string& outError);
    bool GetSystem(std::string& outError);
    bool CreateVRDevice(LUID adapterLuid, std::string& outError);
    bool CreateSession(ID3D11Device* armaDevice, uint32_t width, uint32_t height, std::string& outError);
    bool CreateReferenceSpace(std::string& outError);
    bool CreateSwapchains(uint32_t width, uint32_t height, std::string& outError);

    XrInstance m_instance = XR_NULL_HANDLE;
    XrSystemId m_system = XR_NULL_SYSTEM_ID;
    XrSession m_session = XR_NULL_HANDLE;
    XrSpace m_referenceSpace = XR_NULL_HANDLE;

    bool m_initialized = false;
    bool m_d3d11ExtensionEnabled = false;
    bool m_sessionRunning = false;
    XrSessionState m_sessionState = XR_SESSION_STATE_UNKNOWN;
    std::string m_systemName;

    uint32_t m_viewCount = 0;
    int64_t m_swapchainFormat = 0;
    std::vector<XrViewConfigurationView> m_configViews;
    std::vector<XrView> m_views;
    std::vector<SwapchainData> m_swapchains;

    // VR Device used by OpenXR runtime
    ComPtr<ID3D11Device> m_vrDevice;
    ComPtr<ID3D11DeviceContext> m_vrContext;

    // Stereo reprojection processor
    StereoPipeline m_stereoPipeline;
    bool m_stereoEnabled = true;
    float m_separation = 0.007f;
    float m_convergence = 0.0f;
};