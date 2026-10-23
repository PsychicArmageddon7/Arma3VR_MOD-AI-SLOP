#pragma once
#include "Common.h"

#pragma pack(push, 1)
struct FTData {
    uint32_t DataID;
    int32_t CamWidth;
    int32_t CamHeight;
    float Yaw;
    float Pitch;
    float Roll;
    float X, Y, Z;
    float RawYaw, RawPitch, RawRoll;
    float RawX, RawY, RawZ;
    float X1, Y1, X2, Y2, X3, Y3, X4, Y4;
};

struct FTHeap {
    FTData data;
    int32_t GameID;
    unsigned char table[8];
    int32_t GameID2;
};
#pragma pack(pop)

class FreeTrackProvider {
public:
    FreeTrackProvider() = default;
    ~FreeTrackProvider() { Shutdown(); }

    bool Initialize();
    void Shutdown();
    void Recenter() { m_recenterRequested = true; }
    void UpdatePose(float yawRad, float pitchRad, float rollRad);

    bool IsActive() const { return m_heap != nullptr; }

private:
    HANDLE m_mutex = nullptr;
    HANDLE m_fileMapping = nullptr;
    FTHeap* m_heap = nullptr;

    bool m_hasInitialCenter = false;
    bool m_recenterRequested = false;
    float m_yawOffset = 0.0f;
    float m_pitchOffset = 0.0f;
};