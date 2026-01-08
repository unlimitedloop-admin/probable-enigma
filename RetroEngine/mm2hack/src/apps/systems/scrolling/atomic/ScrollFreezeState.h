//==============================================================================
// 
//  Project: mm2hack
//  ScrollFreezeState.h
// 
//  Manages the freeze state during scrolling.
// 
//==============================================================================
#pragma once

#include <optional>
#include "ScrollTypes.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    // State for scroll freeze frames
    class ScrollFreezeState final
    {
    public:
        static constexpr int kFreezeOnStart = 30;
        static constexpr int kFreezeOnEnd = 30;

        [[nodiscard]] bool IsActive() const noexcept { return _frames > 0; }
        [[nodiscard]] int Frames() const noexcept { return _frames; }

        void BeginStartFreeze() noexcept { _frames = kFreezeOnStart; }
        void BeginEndFreeze() noexcept { _frames = kFreezeOnEnd; }

        // Decrements frames and clears snapshot when done. Returns true if freeze was active on this tick.
        bool Tick() noexcept;

        void SetDrawSnapshot(const PageScroll& s) noexcept { _draw_snapshot = s; }
        void ClearDrawSnapshot() noexcept { _draw_snapshot.reset(); }

        const std::optional<PageScroll>& DrawSnapshot() const noexcept { return _draw_snapshot; }
    
    private:
        std::optional<PageScroll> _draw_snapshot{};
        int _frames{ 0 };
    };
}