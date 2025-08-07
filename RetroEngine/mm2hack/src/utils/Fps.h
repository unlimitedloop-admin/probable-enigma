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
        explicit Fps(int targetFps);

        // Wait for the next frame
        void Wait();
        void Reset();
        double GetActualFps() const;
        void SetTargetFps(int targetFps);

    private:
        using Clock = std::chrono::steady_clock;
        Clock::time_point _lastTime;
        Clock::time_point _fpsTime;
        double _frameDuration;
        int _frameCount;
        mutable double _actualFps;
    };
}