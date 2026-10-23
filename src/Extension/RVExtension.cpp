#include "Common.h"
#include "Extension/RVExtension.h"
#include "Graphics/ArmaGraphics.h"
#include "Graphics/DepthCaptureHook.h"
#include "Tracking/FreeTrackProvider.h"
#include "VR/OpenXRContext.h"

namespace {
    ArmaGraphics g_graphics;
    OpenXRContext g_openXR;
    FreeTrackProvider g_freeTrack;
    std::mutex g_apiMutex;

    void SetOutput(char* output, unsigned int outputSize, const std::string& text) {
        if (output && outputSize > 0) {
            strncpy_s(output, outputSize, text.c_str(), _TRUNCATE);
        }
    }

    bool GetArmaPointers(ID3D11Device*& device, ID3D11DeviceContext*& context, std::string& error) {
        device = nullptr;
        context = nullptr;

        auto lock = g_graphics.AcquireLock();
        if (!lock.IsValid()) {
            error = "RVExtensionGLock returned invalid lock";
            return false;
        }

        const auto* info = g_graphics.GetRenderInfo();
        if (!info || !info->d3dDevice || !info->d3dDeviceContext) {
            error = "Arma render info or device context is null";
            return false;
        }

        device = info->d3dDevice;
        context = info->d3dDeviceContext;
        return true;
    }

    void __stdcall OnArmaGraphicsReset() {
        std::lock_guard<std::mutex> guard(g_apiMutex);
        g_openXR.Shutdown();
    }

    std::string CmdInitialize() {
        std::string err;
        g_freeTrack.Initialize();
        if (!g_graphics.Resolve(err)) return "Graphics resolve failed: " + err;
        if (!g_openXR.Initialize(err)) return "OpenXR init failed: " + err;
        return "OpenXR initialized. " + g_openXR.GetStatus();
    }

    std::string CmdStartSession() {
        if (!g_openXR.IsInitialized()) return "OpenXR must be initialized first";

        std::string err;
        ID3D11Device* dev = nullptr;
        ID3D11DeviceContext* ctx = nullptr;
        if (!GetArmaPointers(dev, ctx, err)) return err;

        if (!g_openXR.StartSession(dev, ctx, err)) return "OpenXR session failed: " + err;

        DepthCaptureHook::Instance().Install(ctx);
        g_graphics.InstallResetHook(&OnArmaGraphicsReset);
        g_freeTrack.Initialize();

        return "Session started. " + g_openXR.GetStatus();
    }

    std::string CmdStepVR() {
        if (!g_openXR.IsSessionCreated()) return "Session not created";

        g_openXR.PollEvents();

        ID3D11Device* dev = nullptr;
        ID3D11DeviceContext* ctx = nullptr;
        std::string err;
        if (!GetArmaPointers(dev, ctx, err)) return "Device error: " + err;

        auto lock = g_graphics.AcquireLock();
        if (!lock.IsValid()) return "Could not acquire Arma graphics lock";

        // Query active color backbuffer
        ComPtr<ID3D11RenderTargetView> currentRtv;
        ctx->OMGetRenderTargets(1, &currentRtv, nullptr);

        ComPtr<ID3D11Texture2D> backbuffer;
        if (currentRtv) {
            ComPtr<ID3D11Resource> res;
            currentRtv->GetResource(&res);
            if (res) res.As(&backbuffer);
        }

        // Retrieve depth texture intercepted by the hook
        ComPtr<ID3D11Texture2D> depthTexture = DepthCaptureHook::Instance().GetCapturedDepthTexture();

        float yaw = 0.0f, pitch = 0.0f, roll = 0.0f;
        std::string diagInfo;
        bool rendered = g_openXR.RenderFrame(
            ctx, backbuffer.Get(), depthTexture.Get(), yaw, pitch, roll, diagInfo, err
        );

        if (!rendered) return "RenderFrame failed: " + err;

        // Feed HMD pose directly to Arma's TrackIR/FreeTrack interface
        g_freeTrack.UpdatePose(yaw, pitch, roll);
        return "OK;" + diagInfo;
    }

    std::string CmdShutdown() {
        ID3D11Device* dev = nullptr;
        ID3D11DeviceContext* ctx = nullptr;
        std::string err;
        if (GetArmaPointers(dev, ctx, err) && ctx) {
            DepthCaptureHook::Instance().Remove(ctx);
        }

        g_freeTrack.Shutdown();
        g_openXR.Shutdown();
        return "Shutdown complete";
    }
}

void __stdcall RVExtensionVersion(char* output, unsigned int outputSize) {
    SetOutput(output, outputSize, "1.0.0");
}

void __stdcall RVExtension(char* output, unsigned int outputSize, const char* function) {
    std::lock_guard<std::mutex> guard(g_apiMutex);
    if (!function) {
        SetOutput(output, outputSize, "No function specified");
        return;
    }

    const std::string cmd(function);

    if (cmd == "ping")        SetOutput(output, outputSize, "Arma3VR extension alive");
    else if (cmd == "initialize")  SetOutput(output, outputSize, CmdInitialize());
    else if (cmd == "session")     SetOutput(output, outputSize, CmdStartSession());
    else if (cmd == "step")        SetOutput(output, outputSize, CmdStepVR());
    else if (cmd == "shutdown")    SetOutput(output, outputSize, CmdShutdown());
    else if (cmd == "recenter") {
        g_freeTrack.Recenter();
        SetOutput(output, outputSize, "Recentered");
    }
    else if (cmd == "mode") {
        bool stereo = g_openXR.ToggleStereoMode();
        SetOutput(output, outputSize, stereo ? "Mode: 3D Stereo" : "Mode: 2D Mono");
    }
    else if (cmd == "sep;+") {
        g_openXR.AdjustSeparation(0.001f);
        SetOutput(output, outputSize, "Separation increased");
    }
    else if (cmd == "sep;-") {
        g_openXR.AdjustSeparation(-0.001f);
        SetOutput(output, outputSize, "Separation decreased");
    }
    else if (cmd == "conv;+") {
        g_openXR.AdjustConvergence(0.02f);
        SetOutput(output, outputSize, "Convergence increased");
    }
    else if (cmd == "conv;-") {
        g_openXR.AdjustConvergence(-0.02f);
        SetOutput(output, outputSize, "Convergence decreased");
    }
    else {
        SetOutput(output, outputSize, "Unknown command: " + cmd);
    }
}