#include "pch.h"

#include "Fps.h"

#include <ratio>
#include <thread>

namespace mm2hack::utils
{
    Fps::Fps(int targetFps)
        : _lastTime(Clock::now()),
        _fpsTime(_lastTime),
        _frameDuration(1000.0 / targetFps),
        _frameCount(0),
        _actualFps(0.0)
    {
    }

    void Fps::Wait()
    {
        auto now = Clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(now - _lastTime).count();

        if (elapsed < _frameDuration)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(_frameDuration - elapsed)));
        }

        _lastTime = Clock::now();

        // FPS calculation.
        ++_frameCount;
        double fpsElapsed = std::chrono::duration<double, std::milli>(_lastTime - _fpsTime).count();
        if (fpsElapsed >= 1000.0)
        {
            _actualFps = (_frameCount * 1000.0) / fpsElapsed;
            _frameCount = 0;
            _fpsTime = _lastTime;
        }
    }

    void Fps::Reset()
    {
        _lastTime = Clock::now();
        _fpsTime = _lastTime;
        _frameCount = 0;
        _actualFps = 0.0;
    }

    double Fps::GetActualFps() const
    {
        return _actualFps;
    }
}