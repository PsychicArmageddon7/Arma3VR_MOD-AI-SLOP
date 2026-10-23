#include "VR/OpenXRContext.h"

#pragma comment(lib, "openxr_loader.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace {
    constexpr const char* kAppName = "Arma3VR";
    constexpr const char* kEngineName = "Arma 3";

    void SetXRError(std::string& error, const char* op, XrResult res) {
        std::ostringstream ss;
        ss << op << " failed: " << OpenXRContext::ResultToString(res) << " (" << res << ")";
        error = ss.str();
    }

    void QuaternionToEuler(const XrQuaternionf& q, float& yaw, float& pitch, float& roll) {
        float sinp = 2.0f * (q.w * q.x - q.y * q.z);
        pitch = (std::abs(sinp) >= 1.0f) ? std::copysign(3.14159265f * 0.5f, sinp) : std::asin(sinp);

        float siny = 2.0f * (q.w * q.y + q.z * q.x);
        float cosy = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
        yaw = std::atan2(siny, cosy);

        float sinr = 2.0f * (q.w * q.z + q.x * q.y);
        float cosr = 1.0f - 2.0f * (q.x * q.x + q.z * q.z);
        roll = std::atan2(sinr, cosr);
    }
}

const char* OpenXRContext::ResultToString(XrResult result) {
    switch (result) {
    case XR_SUCCESS: return "XR_SUCCESS";
    case XR_TIMEOUT_EXPIRED: return "XR_TIMEOUT_EXPIRED";
    case XR_SESSION_LOSS_PENDING: return "XR_SESSION_LOSS_PENDING";
    case XR_ERROR_VALIDATION_FAILURE: return "XR_ERROR_VALIDATION_FAILURE";
    case XR_ERROR_RUNTIME_FAILURE: return "XR_ERROR_RUNTIME_FAILURE";
    case XR_ERROR_OUT_OF_MEMORY: return "XR_ERROR_OUT_OF_MEMORY";
    case XR_ERROR_INITIALIZATION_FAILED: return "XR_ERROR_INITIALIZATION_FAILED";
    case XR_ERROR_FUNCTION_UNSUPPORTED: return "XR_ERROR_FUNCTION_UNSUPPORTED";
    case XR_ERROR_EXTENSION_NOT_PRESENT: return "XR_ERROR_EXTENSION_NOT_PRESENT";
    case XR_ERROR_SESSION_RUNNING: return "XR_ERROR_SESSION_RUNNING";
    case XR_ERROR_SESSION_NOT_RUNNING: return "XR_ERROR_SESSION_NOT_RUNNING";
    case XR_ERROR_SESSION_LOST: return "XR_ERROR_SESSION_LOST";
    default: return "XR_UNKNOWN_RESULT";
    }
}

bool OpenXRContext::Initialize(std::string& outError) {
    Shutdown();

    if (!CreateInstance(outError)) return false;
    if (!GetSystem(outError)) {
        Shutdown();
        return false;
    }

    m_initialized = true;
    return true;
}

bool OpenXRContext::CreateInstance(std::string& outError) {
    uint32_t extCount = 0;
    xrEnumerateInstanceExtensionProperties(nullptr, 0, &extCount, nullptr);

    std::vector<XrExtensionProperties> extensions(extCount, { XR_TYPE_EXTENSION_PROPERTIES });
    xrEnumerateInstanceExtensionProperties(nullptr, extCount, &extCount, extensions.data());

    bool hasD3D11 = false;
    for (const auto& ext : extensions) {
        if (strcmp(ext.extensionName, XR_KHR_D3D11_ENABLE_EXTENSION_NAME) == 0) {
            hasD3D11 = true;
            break;
        }
    }

    if (!hasD3D11) {
        outError = "OpenXR runtime does not expose " XR_KHR_D3D11_ENABLE_EXTENSION_NAME;
        return false;
    }

    const char* enabledExts[] = { XR_KHR_D3D11_ENABLE_EXTENSION_NAME };
    XrInstanceCreateInfo createInfo{ XR_TYPE_INSTANCE_CREATE_INFO };
    strncpy_s(createInfo.applicationInfo.applicationName, XR_MAX_APPLICATION_NAME_SIZE, kAppName, _TRUNCATE);
    strncpy_s(createInfo.applicationInfo.engineName, XR_MAX_ENGINE_NAME_SIZE, kEngineName, _TRUNCATE);
    createInfo.applicationInfo.apiVersion = XR_MAKE_VERSION(1, 0, 0);
    createInfo.applicationInfo.applicationVersion = 1;
    createInfo.applicationInfo.engineVersion = 1;
    createInfo.enabledExtensionCount = 1;
    createInfo.enabledExtensionNames = enabledExts;

    XrResult res = xrCreateInstance(&createInfo, &m_instance);
    if (XR_FAILED(res)) {
        SetXRError(outError, "xrCreateInstance", res);
        return false;
    }

    m_d3d11ExtensionEnabled = true;
    return true;
}

bool OpenXRContext::GetSystem(std::string& outError) {
    XrSystemGetInfo sysInfo{ XR_TYPE_SYSTEM_GET_INFO };
    sysInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

    XrResult res = xrGetSystem(m_instance, &sysInfo, &m_system);
    if (XR_FAILED(res)) {
        SetXRError(outError, "xrGetSystem", res);
        return false;
    }

    XrSystemProperties props{ XR_TYPE_SYSTEM_PROPERTIES };
    if (XR_SUCCEEDED(xrGetSystemProperties(m_instance, m_system, &props))) {
        m_systemName = props.systemName;
    }
    return true;
}

bool OpenXRContext::GetD3D11Requirements(XrGraphicsRequirementsD3D11KHR& req, std::string& outError) {
    if (!m_initialized || m_instance == XR_NULL_HANDLE) {
        outError = "OpenXR instance not initialized";
        return false;
    }

    PFN_xrGetD3D11GraphicsRequirementsKHR pfnGetD3D11Req = nullptr;
    xrGetInstanceProcAddr(
        m_instance,
        "xrGetD3D11GraphicsRequirementsKHR",
        reinterpret_cast<PFN_xrVoidFunction*>(&pfnGetD3D11Req)
    );

    if (!pfnGetD3D11Req) {
        outError = "Could not locate xrGetD3D11GraphicsRequirementsKHR";
        return false;
    }

    req = XrGraphicsRequirementsD3D11KHR{ XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR };
    XrResult res = pfnGetD3D11Req(m_instance, m_system, &req);
    if (XR_FAILED(res)) {
        SetXRError(outError, "xrGetD3D11GraphicsRequirementsKHR", res);
        return false;
    }
    return true;
}

bool OpenXRContext::CreateVRDevice(LUID adapterLuid, std::string& outError) {
    if (m_vrDevice) return true;

    ComPtr<IDXGIFactory1> factory;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) {
        outError = "CreateDXGIFactory1 failed";
        return false;
    }

    ComPtr<IDXGIAdapter1> targetAdapter;
    ComPtr<IDXGIAdapter1> adapter;
    UINT idx = 0;
    while (factory->EnumAdapters1(idx++, &adapter) != DXGI_ERROR_NOT_FOUND) {
        DXGI_ADAPTER_DESC1 desc{};
        adapter->GetDesc1(&desc);
        if (desc.AdapterLuid.LowPart == adapterLuid.LowPart && desc.AdapterLuid.HighPart == adapterLuid.HighPart) {
            targetAdapter = adapter;
            break;
        }
    }

    if (!targetAdapter) {
        outError = "Matching DXGI adapter for OpenXR LUID not found";
        return false;
    }

    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
    D3D_FEATURE_LEVEL actualLevel;

    HRESULT hr = D3D11CreateDevice(
        targetAdapter.Get(),
        D3D_DRIVER_TYPE_UNKNOWN,
        nullptr,
        flags,
        levels,
        2,
        D3D11_SDK_VERSION,
        &m_vrDevice,
        &actualLevel,
        &m_vrContext
    );

    if (FAILED(hr)) {
        outError = "D3D11CreateDevice failed on OpenXR adapter";
        return false;
    }
    return true;
}

bool OpenXRContext::StartSession(ID3D11Device* device, ID3D11DeviceContext* context, std::string& outError) {
    if (!m_initialized) {
        outError = "OpenXR has not been initialized";
        return false;
    }

    uint32_t width = 1920;
    uint32_t height = 1080;

    if (context) {
        ComPtr<ID3D11RenderTargetView> rtv;
        context->OMGetRenderTargets(1, &rtv, nullptr);
        if (rtv) {
            ComPtr<ID3D11Resource> res;
            rtv->GetResource(&res);
            ComPtr<ID3D11Texture2D> tex;
            if (res && SUCCEEDED(res.As(&tex))) {
                D3D11_TEXTURE2D_DESC d{};
                tex->GetDesc(&d);
                if (d.Width > 0 && d.Height > 0) {
                    width = d.Width;
                    height = d.Height;
                }
            }
        }
    }

    return CreateSession(device, width, height, outError);
}

bool OpenXRContext::CreateSession(ID3D11Device* armaDevice, uint32_t width, uint32_t height, std::string& outError) {
    XrGraphicsRequirementsD3D11KHR req{ XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR };
    if (!GetD3D11Requirements(req, outError)) return false;

    if (!CreateVRDevice(req.adapterLuid, outError)) {
        m_vrDevice = armaDevice;
        armaDevice->GetImmediateContext(&m_vrContext);
    }

    XrGraphicsBindingD3D11KHR binding{ XR_TYPE_GRAPHICS_BINDING_D3D11_KHR };
    binding.device = m_vrDevice.Get();

    XrSessionCreateInfo sInfo{ XR_TYPE_SESSION_CREATE_INFO };
    sInfo.systemId = m_system;
    sInfo.next = &binding;

    XrResult res = xrCreateSession(m_instance, &sInfo, &m_session);
    if (XR_FAILED(res)) {
        SetXRError(outError, "xrCreateSession", res);
        return false;
    }

    if (!CreateReferenceSpace(outError)) return false;
    if (!CreateSwapchains(width, height, outError)) return false;

    return true;
}

bool OpenXRContext::CreateReferenceSpace(std::string& outError) {
    XrReferenceSpaceCreateInfo spaceInfo{ XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
    spaceInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
    spaceInfo.poseInReferenceSpace = { { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f } };

    XrResult res = xrCreateReferenceSpace(m_session, &spaceInfo, &m_referenceSpace);
    if (XR_FAILED(res)) {
        SetXRError(outError, "xrCreateReferenceSpace", res);
        return false;
    }
    return true;
}

bool OpenXRContext::CreateSwapchains(uint32_t width, uint32_t height, std::string& outError) {
    uint32_t viewCount = 0;
    xrEnumerateViewConfigurationViews(m_instance, m_system, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, 0, &viewCount, nullptr);
    m_configViews.resize(viewCount, { XR_TYPE_VIEW_CONFIGURATION_VIEW });
    xrEnumerateViewConfigurationViews(m_instance, m_system, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, viewCount, &viewCount, m_configViews.data());

    m_viewCount = viewCount;
    m_views.resize(viewCount, { XR_TYPE_VIEW });

    uint32_t fmtCount = 0;
    xrEnumerateSwapchainFormats(m_session, 0, &fmtCount, nullptr);
    std::vector<int64_t> formats(fmtCount);
    xrEnumerateSwapchainFormats(m_session, fmtCount, &fmtCount, formats.data());

    int64_t chosenFormat = formats[0];
    for (int64_t f : formats) {
        if (f == DXGI_FORMAT_R8G8B8A8_UNORM) {
            chosenFormat = f;
            break;
        }
    }
    m_swapchainFormat = chosenFormat;
    m_swapchains.resize(m_viewCount);

    for (uint32_t i = 0; i < m_viewCount; ++i) {
        XrSwapchainCreateInfo scInfo{ XR_TYPE_SWAPCHAIN_CREATE_INFO };
        scInfo.usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT | XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_TRANSFER_DST_BIT;
        scInfo.format = chosenFormat;
        scInfo.sampleCount = 1;
        scInfo.width = width;
        scInfo.height = height;
        scInfo.faceCount = 1;
        scInfo.arraySize = 1;
        scInfo.mipCount = 1;

        XrResult res = xrCreateSwapchain(m_session, &scInfo, &m_swapchains[i].handle);
        if (XR_FAILED(res)) {
            SetXRError(outError, "xrCreateSwapchain", res);
            return false;
        }

        m_swapchains[i].width = width;
        m_swapchains[i].height = height;

        uint32_t imgCount = 0;
        xrEnumerateSwapchainImages(m_swapchains[i].handle, 0, &imgCount, nullptr);
        m_swapchains[i].images.resize(imgCount, { XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR });
        xrEnumerateSwapchainImages(
            m_swapchains[i].handle,
            imgCount,
            &imgCount,
            reinterpret_cast<XrSwapchainImageBaseHeader*>(m_swapchains[i].images.data())
        );
    }
    return true;
}

void OpenXRContext::PollEvents() {
    if (m_instance == XR_NULL_HANDLE) return;

    XrEventDataBuffer buffer{ XR_TYPE_EVENT_DATA_BUFFER };
    while (xrPollEvent(m_instance, &buffer) == XR_SUCCESS) {
        if (buffer.type == XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED) {
            auto* sc = reinterpret_cast<XrEventDataSessionStateChanged*>(&buffer);
            m_sessionState = sc->state;

            if (m_sessionState == XR_SESSION_STATE_READY) {
                XrSessionBeginInfo bInfo{ XR_TYPE_SESSION_BEGIN_INFO };
                bInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
                if (XR_SUCCEEDED(xrBeginSession(m_session, &bInfo))) {
                    m_sessionRunning = true;
                }
            }
            else if (m_sessionState == XR_SESSION_STATE_STOPPING) {
                xrEndSession(m_session);
                m_sessionRunning = false;
            }
            else if (m_sessionState == XR_SESSION_STATE_LOSS_PENDING || m_sessionState == XR_SESSION_STATE_EXITING) {
                m_sessionRunning = false;
            }
        }
        buffer = { XR_TYPE_EVENT_DATA_BUFFER };
    }
}

bool OpenXRContext::RenderFrame(
    ID3D11DeviceContext* context,
    ID3D11Texture2D* sourceTexture,
    ID3D11Texture2D* depthTexture,
    float& outYaw, float& outPitch, float& outRoll,
    std::string& diagInfo, std::string& outError)
{
    PollEvents();
    if (!m_sessionRunning) {
        outError = "OpenXR session is not running";
        return false;
    }

    XrFrameWaitInfo waitInfo{ XR_TYPE_FRAME_WAIT_INFO };
    XrFrameState frameState{ XR_TYPE_FRAME_STATE };
    if (XR_FAILED(xrWaitFrame(m_session, &waitInfo, &frameState))) return false;

    XrFrameBeginInfo beginInfo{ XR_TYPE_FRAME_BEGIN_INFO };
    if (XR_FAILED(xrBeginFrame(m_session, &beginInfo))) return false;

    for (uint32_t i = 0; i < m_viewCount; ++i) {
        m_views[i].type = XR_TYPE_VIEW;
        m_views[i].next = nullptr;
    }

    XrViewLocateInfo locateInfo{ XR_TYPE_VIEW_LOCATE_INFO };
    locateInfo.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
    locateInfo.displayTime = frameState.predictedDisplayTime;
    locateInfo.space = m_referenceSpace;

    XrViewState viewState{ XR_TYPE_VIEW_STATE };
    uint32_t outViews = 0;
    if (XR_SUCCEEDED(xrLocateViews(m_session, &locateInfo, &viewState, m_viewCount, &outViews, m_views.data()))) {
        if (viewState.viewStateFlags & XR_VIEW_STATE_ORIENTATION_VALID_BIT) {
            QuaternionToEuler(m_views[0].pose.orientation, outYaw, outPitch, outRoll);
        }
    }

    // Process Stereo Reprojection on Arma's DirectX Context
    if (sourceTexture && m_vrContext) {
        D3D11_TEXTURE2D_DESC srcDesc{};
        sourceTexture->GetDesc(&srcDesc);

        ComPtr<ID3D11Device> armaDevice;
        context->GetDevice(&armaDevice);

        if (m_stereoPipeline.EnsureResources(armaDevice.Get(), m_vrDevice.Get(), srcDesc.Width, srcDesc.Height)) {
            m_stereoPipeline.ProcessFrame(
                context, sourceTexture, depthTexture, m_stereoEnabled, m_separation, m_convergence
            );
        }
    }

    std::vector<XrCompositionLayerBaseHeader*> layers;
    XrCompositionLayerProjection layer{ XR_TYPE_COMPOSITION_LAYER_PROJECTION };
    std::vector<XrCompositionLayerProjectionView> projViews(m_viewCount);

    for (uint32_t i = 0; i < m_viewCount; ++i) {
        XrSwapchainImageAcquireInfo acqInfo{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
        uint32_t imgIndex = 0;
        xrAcquireSwapchainImage(m_swapchains[i].handle, &acqInfo, &imgIndex);

        XrSwapchainImageWaitInfo waitImgInfo{ XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
        waitImgInfo.timeout = XR_INFINITE_DURATION;
        xrWaitSwapchainImage(m_swapchains[i].handle, &waitImgInfo);

        // Blit stereo output directly to VR Swapchain
        ID3D11Texture2D* target = m_swapchains[i].images[imgIndex].texture;
        ID3D11Texture2D* eyeSource = (i == 0) ? m_stereoPipeline.GetLeftEyeVRTexture() : m_stereoPipeline.GetRightEyeVRTexture();

        if (target && eyeSource && m_vrContext) {
            m_vrContext->CopyResource(target, eyeSource);
            m_vrContext->Flush();
        }

        XrSwapchainImageReleaseInfo relInfo{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
        xrReleaseSwapchainImage(m_swapchains[i].handle, &relInfo);

        // Calculate Monoscopic Center Pose and Aspect
        XrPosef monoPose = m_views[i].pose;
        if (m_viewCount >= 2) {
            monoPose.position.x = (m_views[0].pose.position.x + m_views[1].pose.position.x) * 0.5f;
            monoPose.position.y = (m_views[0].pose.position.y + m_views[1].pose.position.y) * 0.5f;
            monoPose.position.z = (m_views[0].pose.position.z + m_views[1].pose.position.z) * 0.5f;
            monoPose.orientation = m_views[0].pose.orientation;
        }

        float maxHalfHoriz = (std::max)(std::abs(m_views[0].fov.angleLeft), std::abs(m_views[0].fov.angleRight));
        if (maxHalfHoriz <= 0.01f) maxHalfHoriz = 0.872665f;

        float aspect = 0.5625f;
        if (sourceTexture) {
            D3D11_TEXTURE2D_DESC d{};
            sourceTexture->GetDesc(&d);
            if (d.Width > 0 && d.Height > 0) aspect = static_cast<float>(d.Height) / static_cast<float>(d.Width);
        }
        float halfVert = maxHalfHoriz * aspect;

        projViews[i].type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
        projViews[i].pose = monoPose;
        projViews[i].fov = { -maxHalfHoriz, maxHalfHoriz, halfVert, -halfVert };
        projViews[i].subImage.swapchain = m_swapchains[i].handle;
        projViews[i].subImage.imageRect.offset = { 0, 0 };
        projViews[i].subImage.imageRect.extent = { m_swapchains[i].width, m_swapchains[i].height };
    }

    layer.space = m_referenceSpace;
    layer.viewCount = m_viewCount;
    layer.views = projViews.data();
    layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader*>(&layer));

    XrFrameEndInfo endInfo{ XR_TYPE_FRAME_END_INFO };
    endInfo.displayTime = frameState.predictedDisplayTime;
    endInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
    endInfo.layerCount = static_cast<uint32_t>(layers.size());
    endInfo.layers = layers.empty() ? nullptr : layers.data();

    XrResult endRes = xrEndFrame(m_session, &endInfo);
    if (XR_FAILED(endRes)) {
        SetXRError(outError, "xrEndFrame", endRes);
        return false;
    }

    diagInfo = m_stereoEnabled ? "stereo_depth" : "mono_direct";
    return true;
}

void OpenXRContext::Shutdown() {
    m_sessionRunning = false;
    m_sessionState = XR_SESSION_STATE_UNKNOWN;

    m_stereoPipeline.Reset();
    m_vrContext.Reset();
    m_vrDevice.Reset();

    for (auto& sc : m_swapchains) {
        if (sc.handle != XR_NULL_HANDLE) xrDestroySwapchain(sc.handle);
    }
    m_swapchains.clear();
    m_configViews.clear();
    m_views.clear();

    if (m_referenceSpace != XR_NULL_HANDLE) {
        xrDestroySpace(m_referenceSpace);
        m_referenceSpace = XR_NULL_HANDLE;
    }
    if (m_session != XR_NULL_HANDLE) {
        xrDestroySession(m_session);
        m_session = XR_NULL_HANDLE;
    }
    if (m_instance != XR_NULL_HANDLE) {
        xrDestroyInstance(m_instance);
        m_instance = XR_NULL_HANDLE;
    }

    m_system = XR_NULL_SYSTEM_ID;
    m_systemName.clear();
    m_viewCount = 0;
    m_swapchainFormat = 0;
    m_d3d11ExtensionEnabled = false;
    m_initialized = false;
}

std::string OpenXRContext::GetStatus() const {
    std::ostringstream ss;
    ss << "initialized=" << (m_initialized ? 1 : 0)
        << ";instance=" << (m_instance != XR_NULL_HANDLE ? 1 : 0)
        << ";session=" << (m_session != XR_NULL_HANDLE ? 1 : 0)
        << ";systemName=" << m_systemName
        << ";views=" << m_viewCount;
    return ss.str();
}