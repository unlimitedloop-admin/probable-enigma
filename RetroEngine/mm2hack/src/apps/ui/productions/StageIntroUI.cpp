#include "pch.h"

#include "StageIntroUI.h"

#include "apps/runtime/GameContext.h"

namespace mm2hack::apps::ui::productions
{
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
}