//==============================================================================
// 
//  Project: mm2hack
//  FixedScrollDriver.h
// 
//  Manages fixed scroll driving logic.
// 
//==============================================================================
#pragma once

#include <optional>
#include "PageScrollAnimator.h"
#include "ScrollFreezeState.h"
#include "ScrollTypes.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    struct Camera;
    struct ScrollEffect;
    class ScrollNeighborResolver;

    // Fixed scroll management
    class FixedScrollDriver final
    {
    public:
        FixedScrollDriver(PageScrollAnimator& animator, const ScrollNeighborResolver& resolver) noexcept
            : _animator(animator)
            , _resolver(resolver)
        {
        }

        [[nodiscard]] bool IsLocked() const noexcept
        {
            return _animator.Active() || _pending.has_value();
        }
        
        // Request fixed scroll. Returns false if rejected (no neighbor / not allowed / already animating)
        bool Request(const FixedScrollRequest& req) noexcept;
        // Update per frame. Returns true if fixed scroll is active (freeze/anim/pending)
        bool Update(int page_w, int page_h, Camera& cam, std::size_t& page_index, ScrollFreezeState& freeze, ScrollEffect& out_fx) noexcept;

    private:
        // Attempt to start fixed scroll animation. Returns true if started
        bool tryStart_(const FixedScrollRequest& req, std::size_t& page_index, int page_w, int page_h, Camera& cam) noexcept;
        // Tick animation
        void tickAnim_(int page_w, int page_h, Camera& cam, std::size_t& page_index, ScrollFreezeState& freeze, ScrollEffect& out_fx) noexcept;
        // Finish animation
        void finish_(double need, Camera& cam, std::size_t& page_index, ScrollFreezeState& freeze) noexcept;

    private:
        PageScrollAnimator& _animator;
        const ScrollNeighborResolver& _resolver;

        std::optional<FixedScrollRequest> _pending{};
        double _carry_total_px{ 0.0 };
    };
}