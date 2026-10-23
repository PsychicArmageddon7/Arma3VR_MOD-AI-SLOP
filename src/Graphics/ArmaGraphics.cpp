#include "Graphics/ArmaGraphics.h"

const void* ArmaGraphics::FindGameFunction(const char* name) {
    if (!name) return nullptr;
    HMODULE armaModule = GetModuleHandleW(nullptr);
    return armaModule ? reinterpret_cast<const void*>(GetProcAddress(armaModule, name)) : nullptr;
}

bool ArmaGraphics::Resolve(std::string& outError) {
    if (m_resolved) return true;

    m_gData = reinterpret_cast<RVExtensionGDataPtr>(const_cast<void*>(FindGameFunction("RVExtensionGData")));
    m_gLock = reinterpret_cast<RVExtensionGLockProc>(const_cast<void*>(FindGameFunction("RVExtensionGLock")));
    m_getProjectionViewTransform = reinterpret_cast<RVGetProjectionViewTransformProc>(const_cast<void*>(FindGameFunction("RVGetProjectionViewTransform")));
    m_setResetHook = reinterpret_cast<RVExtensionGSetResetHookProc>(const_cast<void*>(FindGameFunction("RVExtensionGSetResetHook")));

    if (!m_gData || !m_gLock || !m_getProjectionViewTransform || !m_setResetHook) {
        outError = "Failed to resolve one or more Arma graphics entrypoints.";
        return false;
    }

    m_resolved = true;
    return true;
}

ArmaGraphics::ScopedLock ArmaGraphics::AcquireLock() const {
    return m_gLock ? ScopedLock(m_gLock()) : ScopedLock();
}

const RVExtensionRenderInfo* ArmaGraphics::GetRenderInfo() const {
    return m_gData ? *m_gData : nullptr;
}

bool ArmaGraphics::GetAdapterLuid(LUID& outLuid, std::string& outAdapterName, std::string& outError) const {
    outLuid = LUID{};
    outAdapterName.clear();

    const auto* info = GetRenderInfo();
    if (!info || !info->d3dDevice) {
        outError = "Arma graphics device is null or unavailable";
        return false;
    }

    ComPtr<IDXGIDevice> dxgiDevice;
    if (FAILED(info->d3dDevice->QueryInterface(IID_PPV_ARGS(&dxgiDevice)))) {
        outError = "QueryInterface(IDXGIDevice) failed";
        return false;
    }

    ComPtr<IDXGIAdapter> adapter;
    if (FAILED(dxgiDevice->GetAdapter(&adapter))) {
        outError = "IDXGIDevice::GetAdapter failed";
        return false;
    }

    DXGI_ADAPTER_DESC desc{};
    if (FAILED(adapter->GetDesc(&desc))) {
        outError = "IDXGIAdapter::GetDesc failed";
        return false;
    }

    outLuid = desc.AdapterLuid;
    if (desc.Description[0] != L'\0') {
        int len = WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, nullptr, 0, nullptr, nullptr);
        if (len > 0) {
            std::vector<char> buf(len);
            WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, buf.data(), len, nullptr, nullptr);
            outAdapterName = buf.data();
        }
    }
    return true;
}

bool ArmaGraphics::GetProjectionViewTransform(ProjectionViewTransform& outTransform) const {
    if (!m_getProjectionViewTransform) return false;
    m_getProjectionViewTransform(outTransform);
    return true;
}

bool ArmaGraphics::InstallResetHook(ResetHook hook) const {
    if (!m_setResetHook) return false;
    m_setResetHook(hook);
    return true;
}