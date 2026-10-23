#include "Tracking/FreeTrackProvider.h"

bool FreeTrackProvider::Initialize() {
    if (m_fileMapping != nullptr) {
        return true;
    }

    m_mutex = CreateMutexA(nullptr, FALSE, "FT_Mutext");
    m_fileMapping = CreateFileMappingA(
        INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(FTHeap), "FT_SharedMem"
    );

    if (m_fileMapping == nullptr) {
        Shutdown();
        return false;
    }

    m_heap = reinterpret_cast<FTHeap*>(
        MapViewOfFile(m_fileMapping, FILE_MAP_WRITE, 0, 0, sizeof(FTHeap))
        );

    if (m_heap == nullptr) {
        Shutdown();
        return false;
    }

    ZeroMemory(m_heap, sizeof(FTHeap));
    m_heap->data.CamWidth = 640;
    m_heap->data.CamHeight = 480;
    return true;
}

void FreeTrackProvider::Shutdown() {
    m_hasInitialCenter = false;
    m_recenterRequested = false;
    m_yawOffset = 0.0f;
    m_pitchOffset = 0.0f;

    if (m_heap) {
        UnmapViewOfFile(m_heap);
        m_heap = nullptr;
    }
    if (m_fileMapping) {
        CloseHandle(m_fileMapping);
        m_fileMapping = nullptr;
    }
    if (m_mutex) {
        CloseHandle(m_mutex);
        m_mutex = nullptr;
    }
}

void FreeTrackProvider::UpdatePose(float yawRad, float pitchRad, float /*rollRad*/) {
    if (m_heap == nullptr) return;

    if (!m_hasInitialCenter || m_recenterRequested) {
        m_yawOffset = yawRad;
        m_pitchOffset = pitchRad;
        m_hasInitialCenter = true;
        m_recenterRequested = false;
    }

    float relativeYaw = yawRad - m_yawOffset;
    float relativePitch = pitchRad - m_pitchOffset;

    // Normalize to [-PI, +PI]
    constexpr float kPi = 3.14159265f;
    constexpr float kTwoPi = 6.2831853f;
    while (relativeYaw > kPi) relativeYaw -= kTwoPi;
    while (relativeYaw < -kPi) relativeYaw += kTwoPi;

    constexpr float kSensitivity = 0.25f;
    float finalYaw = relativeYaw * kSensitivity;
    float finalPitch = relativePitch * kSensitivity;

    m_heap->data.DataID++;
    m_heap->data.Yaw = finalYaw;
    m_heap->data.Pitch = finalPitch;
    m_heap->data.Roll = 0.0f;

    m_heap->data.RawYaw = finalYaw;
    m_heap->data.RawPitch = finalPitch;
    m_heap->data.RawRoll = 0.0f;
}