//==============================================================================
// 
//  Project: mm2hack
//  ITimeController.h
// 
//  Framework for time control in games.
// 
//==============================================================================
#pragma once

#include <cstdint>

namespace mm2hack::core::assembly
{
    /// Abstract interface for time control, Corresponds to Fixed Delta and Step One Frame during pause
    class ITimeController
    {
    public:
        using FSeconds = double;          ///< Floating-point representation of seconds. Ex: 1/60.0

        virtual ~ITimeController() = default;

        // ====== Lifecycle (Frame Boundaries) ======
        /// Call this at the beginning of the frame. Updates internal state (determining delta t, consuming step requests, etc.)
        virtual void BeginFrame() noexcept = 0;
        /// Call this at the end of the frame. Performs post-processing such as frame counter updates
        virtual void EndFrame() noexcept = 0;

        // ====== Pause/Step ======
        /// Pause the time controller (after this, DeltaSeconds() usually returns 0)
        virtual void Pause() noexcept = 0;
        /// Resume the time controller
        virtual void Resume() noexcept = 0;
        /// Check if currently paused
        [[nodiscard]] virtual bool IsPaused() const noexcept = 0;

        /// Request to supply fixed delta t for "just the next frame". Effective even when paused
        /// Specifically, this request is consumed at the next BeginFrame(), causing DeltaSeconds() to equal FixedDeltaSeconds()
        virtual void StepOneFrame() noexcept = 0;

        // ====== Fixed delta t, Time Scale ======
        /// Set fixed delta t (seconds). Ex: 1/60.0
        virtual void SetFixedDeltaSeconds(FSeconds dt) noexcept = 0;
        /// Get fixed delta t (seconds)
        [[nodiscard]] virtual FSeconds GetFixedDeltaSeconds() const noexcept = 0;
        /// Convenience function: Set fixed delta t from FPS. Ex: SetFixedDeltaByFps(60) -> 1/60 seconds
        virtual void SetFixedDeltaByFps(std::uint32_t fps) noexcept = 0;
        /// Time scale (>=0). 1.0=normal, 0.5=half speed. It is recommended to implement supplying fixed delta t once during StepOneFrame regardless of scale
        virtual void SetTimeScale(FSeconds scale) noexcept = 0;
        /// Get current time scale
        [[nodiscard]] virtual FSeconds GetTimeScale() const noexcept = 0;

        // ====== Current Frame delta t, Statistics ======
        /// Current frame's delta t (seconds). Usually returns 0 when IsPaused()==true, returns FixedDelta when StepOneFrame is specified
        [[nodiscard]] virtual FSeconds DeltaSeconds() const noexcept = 0;
        /// Number of frames elapsed (implementation that counts only frames where updates occurred is recommended)
        [[nodiscard]] virtual std::uint64_t FrameCounter() const noexcept = 0;

        // ====== Follow FPS Mode ======
        // When enabled, DeltaSeconds() returns FixedDeltaSeconds() even when not stepping one frame
        virtual void EnableFollowFps(bool enabled) noexcept = 0;
        // Check if Follow FPS mode is enabled
        [[nodiscard]] virtual bool IsFollowFpsEnabled() const noexcept = 0;

        // Reset (initializes pause state, counters, step requests, etc.)
        virtual void Reset() noexcept = 0;

        // Reset frame counter for play mode
        virtual void ResetPlayFrameCounter() noexcept = 0;
        // Step frame counter for play mode
        virtual void IncrementPlayFrameCounter() noexcept = 0;
        // Get frame counter for play mode
        [[nodiscard]] virtual std::uint64_t GetPlayFrameCounter() const noexcept = 0;
    };
}