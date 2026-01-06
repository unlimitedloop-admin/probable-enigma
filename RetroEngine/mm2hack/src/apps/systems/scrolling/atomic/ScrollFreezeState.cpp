#include "pch.h"

#include "ScrollFreezeState.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    bool ScrollFreezeState::Tick() noexcept
    {
        if (_frames <= 0)
        {
            return false;
        }

        --_frames;

        if (_frames == 0)
        {
            _draw_snapshot.reset();
        }

        return true;
    }
}