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

    class IFixedNeighborResolver
    {
    public:
        virtual ~IFixedNeighborResolver() = default;
        virtual std::optional<std::size_t> ResolveFixedNeighbor(PageScroll::Dir dir, std::size_t from) const = 0;
    };

    class FixedScrollDriver final
    {
    public:
        explicit FixedScrollDriver(PageScrollAnimator& animator, IFixedNeighborResolver& resolver) noexcept
            : _animator(animator), _resolver(resolver)
        {
        }

        [[nodiscard]] bool IsLocked() const noexcept { return _animator.Active() || _pending.has_value(); }

        bool Request(const FixedScrollRequest& req) noexcept;

        // Returns true if it consumed the frame (freeze or anim or started pending).
        bool Update(int page_w, int page_h, Camera& cam, std::size_t& page_index, ScrollFreezeState& freeze, ScrollEffect& out_fx) noexcept;

    private:
        bool tryStart_(const FixedScrollRequest& req, std::size_t& page_index, int page_w, int page_h, Camera& cam) noexcept;
        void tickAnim_(int page_w, int page_h, Camera& cam, std::size_t& page_index, ScrollFreezeState& freeze, ScrollEffect& out_fx) noexcept;
        void finish_(double need, Camera& cam, std::size_t& page_index, ScrollFreezeState& freeze) noexcept;

    private:
        PageScrollAnimator& _animator;
        IFixedNeighborResolver& _resolver;

        std::optional<FixedScrollRequest> _pending{};
        double _carry_total_px{ 0.0 };
    };
}