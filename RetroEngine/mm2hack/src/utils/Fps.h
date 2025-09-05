//==============================================================================
//
//  Project: mm2hack
//  Fps.h
//
//  The controlling frame rate of the game.
//
//==============================================================================
#pragma once

#include <chrono>

namespace mm2hack::utils
{
    // Determines the frame rate to ensure the game runs at a consistent speed
    class Fps
    {
    public:
        Fps();

        // Wait for the next frame
        void Wait();
        // Reset the internal timer
        void Reset();

        // Get the actual frame rate
        double GetActualFps() const;
        // Set the target frame rate
        void SetTargetFps(int targetFps);
        // Get the time elapsed since the last frame in seconds
        float GetDeltaSeconds() const;

    private:
        using Clock = std::chrono::steady_clock;
        Clock::time_point _lastTime;
        Clock::time_point _fpsTime;
        double _frameDuration;
        int _frameCount;
        mutable double _actualFps;

        void LoadIniFps();  // Load target FPS from ini file or use SystemConfig value
    };
}