#pragma once
#include "Common.h"

struct RVExtensionRenderInfo {
    ID3D11Device* d3dDevice;
    ID3D11DeviceContext* d3dDeviceContext;
};

struct RVExtensionGraphicsLockGuard {
    virtual void ReleaseLock() const = 0;
    virtual ~RVExtensionGraphicsLockGuard() = default;
};

struct ProjectionViewTransform {
    float projection[4][4];
    float view[4][4];
};

class ArmaGraphics {
public:
    using ResetHook = void(__stdcall*)();

    class ScopedLock {
    public:
        ScopedLock() = default;
        explicit ScopedLock(RVExtensionGraphicsLockGuard* guard) : m_guard(guard) {}
        ~ScopedLock() { Release(); }

        ScopedLock(const ScopedLock&) = delete;
        ScopedLock& operator=(const ScopedLock&) = delete;

        ScopedLock(ScopedLock&& other) noexcept : m_guard(other.m_guard) {
            other.m_guard = nullptr;
        }

        ScopedLock& operator=(ScopedLock&& other) noexcept {
            if (this != &other) {
                Release();
                m_guard = other.m_guard;
                other.m_guard = nullptr;
            }
            return *this;
        }

        bool IsValid() const { return m_guard != nullptr; }

    private:
        void Release() {
            if (m_guard) {
                m_guard->ReleaseLock();
                m_guard = nullptr;
            }
        }
        RVExtensionGraphicsLockGuard* m_guard = nullptr;
    };

    bool Resolve(std::string& outError);
    bool IsResolved() const { return m_resolved; }

    ScopedLock AcquireLock() const;
    const RVExtensionRenderInfo* GetRenderInfo() const;
    bool GetAdapterLuid(LUID& outLuid, std::string& outAdapterName, std::string& outError) const;
    bool GetProjectionViewTransform(ProjectionViewTransform& outTransform) const;
    bool InstallResetHook(ResetHook hook) const;

private:
    using RVExtensionGDataPtr = const RVExtensionRenderInfo* const*;
    using RVExtensionGLockProc = RVExtensionGraphicsLockGuard * (__stdcall*)();
    using RVGetProjectionViewTransformProc = void(__stdcall*)(ProjectionViewTransform&);
    using RVExtensionGSetResetHookProc = void(__stdcall*)(ResetHook);

    static const void* FindGameFunction(const char* name);

    RVExtensionGDataPtr m_gData = nullptr;
    RVExtensionGLockProc m_gLock = nullptr;
    RVGetProjectionViewTransformProc m_getProjectionViewTransform = nullptr;
    RVExtensionGSetResetHookProc m_setResetHook = nullptr;
    bool m_resolved = false;
};