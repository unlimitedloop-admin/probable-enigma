#include "pch.h"

#include "StandardTimeController.h"

#include <cstdint>
#include <cstdlib>
#include "core/overlay/PauseManager.h"
#include "ITimeController.h"
#include "utils/Fps.h"

namespace mm2hack::core::assembly
{
    using overlay::PauseManager;

    StandardTimeController::StandardTimeController(utils::Fps* fps, bool callWaitInBeginFrame) noexcept
        : _fps(fps), _callWait(callWaitInBeginFrame)
    {
    }

    void StandardTimeController::BeginFrame() noexcept
    {
        SyncFromFpsTargetIfNeeded();

        // If an Fps instance is provided, wait for the next frame
        if (_fps && _callWait) { _fps->Wait(); }

        const bool paused = PauseManager::IsPaused();

        if (paused)
        {
            if (_stepOnce)
            {
                _delta = _fixedDt * _scale;  // Step one frame with fixed delta time.
                _stepOnce = false;
                ++_advancedFrames;
            }
            else
            {
                _delta = 0.0;
            }
        }
        else
        {
            // Default behavior: use fixed delta time scaled by the time scale factor.
            _delta = _fixedDt * _scale;
            ++_advancedFrames;
        }
    }

    void StandardTimeController::EndFrame() noexcept
    {
        // Here we could add any end-of-frame logic if needed.
    }

    void StandardTimeController::Pause() noexcept
    {
        PauseManager::SetPaused(true);
    }

    void StandardTimeController::Resume() noexcept
    {
        PauseManager::SetPaused(false);
    }

    bool StandardTimeController::IsPaused() const noexcept
    {
        return PauseManager::IsPaused();
    }

    void StandardTimeController::StepOneFrame() noexcept
    {
        _stepOnce = true;
    }

    void StandardTimeController::SetFixedDeltaSeconds(FSeconds dt) noexcept
    {
        _fixedDt = (dt > 0.0) ? dt : 0.0;
        // Sync FPS
        if (_fps && _fixedDt > 0.0)
        {
            const auto fps = static_cast<std::uint32_t>(std::clamp(1.0 / _fixedDt, 1.0, 1000.0));
            _fps->SetTargetFps(static_cast<int>(fps));
        }
    }

    ITimeController::FSeconds StandardTimeController::GetFixedDeltaSeconds() const noexcept
    {
        return _fixedDt;
    }

    void StandardTimeController::SetFixedDeltaByFps(std::uint32_t fps) noexcept
    {
        if (fps == 0) { return; }
        _fixedDt = 1.0 / static_cast<FSeconds>(fps);
        if (_fps) { _fps->SetTargetFps(static_cast<int>(fps)); }
    }

    void StandardTimeController::SetTimeScale(FSeconds scale) noexcept
    {
        _scale = (scale >= 0.0) ? scale : 0.0;
    }

    ITimeController::FSeconds StandardTimeController::GetTimeScale() const noexcept
    {
        return _scale;
    }

    ITimeController::FSeconds StandardTimeController::DeltaSeconds() const noexcept
    {
        return _delta;
    }

    std::uint64_t StandardTimeController::FrameCounter() const noexcept
    {
        return _advancedFrames;
    }

    void StandardTimeController::EnableFollowFps(bool enabled) noexcept
    {
        _followFps = enabled;
    }

    bool StandardTimeController::IsFollowFpsEnabled() const noexcept
    {
        return _followFps;
    }

    void StandardTimeController::Reset() noexcept
    {
        _delta = 0.0;
        _scale = 1.0;
        _stepOnce = false;
        _advancedFrames = 0;
    }

    void StandardTimeController::SyncFromFpsTargetIfNeeded() noexcept
    {
        if (!_fps || !_followFps) return;

        const double targetDt = static_cast<double>(_fps->GetDeltaSeconds());
        if (targetDt > 0.0)
        {
            constexpr double kEps = 1e-6;
            if (std::abs(_fixedDt - targetDt) > kEps)
            {
                _fixedDt = targetDt;
            }
        }
    }
}