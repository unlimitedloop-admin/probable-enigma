#include "pch.h"

#include "Fps.h"

#include <iterator>
#include <ratio>
#include <thread>
#include "config/ConfigUIManager.h"
#include "config/GraphicsConfig.h"
#include "core/ui/GraphicsSettingsUI.h"

namespace mm2hack::utils
{
    Fps::Fps()
        : _lastTime(Clock::now()),
        _fpsTime(_lastTime),
        _frameDuration(1000.0 / 60),
        _frameCount(0),
        _actualFps(0.0)
    {
        LoadIniFps();
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
            // When processing time exceeds 1 second, calculate the actual FPS.
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

    void Fps::SetTargetFps(int targetFps)
    {
        if (targetFps > 0)
        {
            _frameDuration = 1000.0 / targetFps;
        }
        else
        {
            _frameDuration = 0;  // Unlimited
        }
    }

    float Fps::GetDeltaSeconds() const
    {
        return static_cast<float>(_frameDuration) / 1000.0f;
    }

    void Fps::LoadIniFps()
    {
        using namespace config;
        using namespace core::ui;

        int fpsToSet = SystemConfig::kTargetFps;

        GraphicsConfig conf{};
        ConfigUIManager::LoadGraphicsConfig(conf);
        if (conf.fpsLimitIndex >= 0 && conf.fpsLimitIndex < static_cast<int>(std::size(kFramerateOptions)))
        {
            fpsToSet = kFramerateOptions[conf.fpsLimitIndex].targetFps;   // Set the specified FPS limit.
        }
        SetTargetFps(fpsToSet);
    }
}