#include "pch.h"

#include "StageIntroUI.h"

#include <cmath>
#include "apps/runtime/GameContext.h"

namespace mm2hack::apps::ui::productions
{
    bool StageIntroUIState::Save(core::save::StateWriter& writer) const
    {
        return IsValid() && writer.WriteF64(total_duration) &&
            writer.WriteF64(elapsed) && writer.WriteBool(finished);
    }

    bool StageIntroUIState::Load(core::save::StateReader& reader)
    {
        StageIntroUIState loaded{};
        if (!reader.ReadF64(loaded.total_duration) ||
            !reader.ReadF64(loaded.elapsed) ||
            !reader.ReadBool(loaded.finished) || !loaded.IsValid())
        {
            return false;
        }
        *this = loaded;
        return true;
    }

    bool StageIntroUIState::IsValid() const noexcept
    {
        constexpr double kMaximumDurationSeconds = 60.0 * 60.0;
        return std::isfinite(total_duration) && total_duration > 0.0 &&
            total_duration <= kMaximumDurationSeconds &&
            std::isfinite(elapsed) && elapsed >= 0.0 &&
            elapsed <= kMaximumDurationSeconds &&
            finished == (elapsed >= total_duration);
    }

    void StageIntroUI::Begin(double tDuration)
    {
        _totalDuration = tDuration;
        _elapsed = 0.0;
        _finished = false;
    }

    void StageIntroUI::Update(double dt)
    {
        _elapsed += dt;

        // Total duration for the stage intro.
        if (_elapsed >= _totalDuration)
        {
            _finished = true;
        }
    }

    void StageIntroUI::Render() const
    {
        constexpr double kBlinkInterval = 8.0 / 60.0;

        const int phase = static_cast<int>(_elapsed / kBlinkInterval);
        const bool visible = (phase % 2 == 0);

        if (!visible)
            return;

        auto* res = runtime::GameContext::GetInstance().GetResourceManagerPtr();
        auto& fonts = res->GetFontTileManager();
        fonts.DrawTextImage(
            L"READY", static_cast<int>(_readyStringPos.x), static_cast<int>(_readyStringPos.y)
        );
    }

    bool StageIntroUI::IsFinished() const
    {
        return _finished;
    }

    StageIntroUIState StageIntroUI::CaptureState() const noexcept
    {
        return StageIntroUIState{ _totalDuration, _elapsed, _finished };
    }

    bool StageIntroUI::RestoreState(const StageIntroUIState& state) noexcept
    {
        if (!state.IsValid()) return false;
        _totalDuration = state.total_duration;
        _elapsed = state.elapsed;
        _finished = state.finished;
        return true;
    }
}
