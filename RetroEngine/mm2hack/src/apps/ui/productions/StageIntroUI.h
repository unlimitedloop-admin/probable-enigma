//==============================================================================
// 
//  Project: mm2hack
//  StageIntroUI.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include <string>
#include "apps/foundation/math/CoordinateTypes.h"

namespace mm2hack::apps::ui::productions
{
    // Stage introduction UI component
    class StageIntroUI
    {
    public:
        StageIntroUI() noexcept = default;
        ~StageIntroUI() = default;
        StageIntroUI(const StageIntroUI&) = delete;
        StageIntroUI& operator=(const StageIntroUI&) = delete;

        // Begin the stage intro UI
        void Begin(double tDuration);
        // Updates the stage intro UI
        void Update(double dt);
        // Renders the stage intro UI
        void Render() const;
        // Checks if the intro sequence is finished
        bool IsFinished() const;

    private:
        const std::wstring kClassName{ L"StageIntroUI" };

        static constexpr foundation::math::Vec2 _readyStringPos{ 108, 86 };  // Position for the "READY" text (default to the central axis is slightly upward)

        double _totalDuration{ 3.0 };       // Total duration for the stage intro sequence (default to 3 seconds)
        double _elapsed{ 0.0 };             // Elapsed time since the start of the intro sequence
        bool _finished{ false };            // Flag indicating if the intro sequence is finished
    };
}