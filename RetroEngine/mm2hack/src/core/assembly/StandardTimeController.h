//==============================================================================
// 
//  Project: mm2hack
//  StandardTimeController.h
//
//  Implements a standard time controller with fixed timestep and optional FPS limiting.
// 
//==============================================================================
#pragma once

#include "ITimeController.h"

#include <cstdint>
#include <string>
#include "utils/Fps.h"

namespace mm2hack::core::assembly
{
    /// Combines a fixed timestep with an optional FPS limiter
    class StandardTimeController final : public ITimeController
    {
    public:
        using FSeconds = ITimeController::FSeconds;

        // @param fps Non-owning pointer to an Fps instance for frame pacing. If nullptr, no pacing is done.
        explicit StandardTimeController(mm2hack::utils::Fps* fps = nullptr,
            bool callWaitInBeginFrame = false) noexcept;

        // Updated ITimeController
        void BeginFrame() noexcept override;
        void EndFrame() noexcept override;

        // Pause/Resume
        void Pause() noexcept override;
        void Resume() noexcept override;
        [[nodiscard]] bool IsPaused() const noexcept override;

        // Step one frame when paused
        void StepOneFrame() noexcept override;

        // Fixed delta time and time scale
        void SetFixedDeltaSeconds(FSeconds dt) noexcept override;
        [[nodiscard]] FSeconds GetFixedDeltaSeconds() const noexcept override;
        // Set fixed delta time based on FPS
        void SetFixedDeltaByFps(std::uint32_t fps) noexcept override;

        // Time scale (>=0). 1.0 = normal speed, 0.5 = half speed.
        void SetTimeScale(FSeconds scale) noexcept override;
        [[nodiscard]] FSeconds GetTimeScale() const noexcept override;

        // Calculate delta time for the current frame
        [[nodiscard]] FSeconds DeltaSeconds() const noexcept override;
        [[nodiscard]] std::uint64_t FrameCounter() const noexcept override;

        // Follow the target FPS of the Fps instance
        void EnableFollowFps(bool enabled) noexcept override;
        [[nodiscard]] bool IsFollowFpsEnabled() const noexcept override;
        // Reset internal state
        void Reset() noexcept override;
        // Case whether to call Fps::Wait() in BeginFrame()
        void SetCallWaitInBeginFrame(bool enabled) noexcept { _callWait = enabled; }

        // Play frame counter management
        void ResetPlayFrameCounter() noexcept override;
        void IncrementPlayFrameCounter() noexcept override;
        [[nodiscard]] std::uint64_t GetPlayFrameCounter() const noexcept override;

    private:
        const std::wstring kClassName = L"StandardTimeController";

        mm2hack::utils::Fps* _fps;                  // External Fps pointer
        bool _callWait;                             // Call wait in BeginFrame
        bool _followFps{ false };                   // Synchronize target FPS with fixed delta time

        FSeconds _fixedDt{ 1.0 / 60.0 };            // Default to 1/60 second
        FSeconds _delta{ 0.0 };                     // Delta time for the current frame
        FSeconds _scale{ 1.0 };                     // Time scale factor
        bool _stepOnce{ false };                    // Whether to step one frame when paused
        std::uint64_t _advancedFrames{ 0 };         // Count of frames that have advanced
        std::uint64_t _playFrameCounter{ 0 };       // Count of frames played (not paused)

        void SyncFromFpsTargetIfNeeded() noexcept;  // Sync fixed delta time from FPS target if _followFps is true
    };
}