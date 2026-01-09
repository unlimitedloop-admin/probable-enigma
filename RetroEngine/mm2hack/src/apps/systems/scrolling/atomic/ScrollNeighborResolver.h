//==============================================================================
// 
//  Project: mm2hack
//  ScrollNeighborResolver.h
// 
//  Provides a resolver for neighboring pages based on scroll rules.
// 
//==============================================================================
#pragma once

#include <cstdint>
#include <optional>
#include "ScrollTypes.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    class IScrollRuleProvider;

    // Resolver for neighboring pages based on scroll rules
    class ScrollNeighborResolver final
    {
    public:
        explicit ScrollNeighborResolver(const IScrollRuleProvider& rules) noexcept
            : _rules(rules)
        {
        }

        // Neighbor index resolvers directly for each x axis (-1: left, +1: right)
        [[nodiscard]] int ResolveNextIndexX(std::size_t page_index, int dir) const;
        // Neighbor index resolvers directly for each y axis (-1: up, +1: down)
        [[nodiscard]] int ResolveNextIndexY(std::size_t page_index, int dir) const;

        [[nodiscard]] std::optional<std::size_t> ResolveFixedNeighbor(PageScroll::Dir dir, std::size_t from) const;

    private:
        [[nodiscard]] static std::optional<std::size_t> roomToIndex_(const IScrollRuleProvider& rules, int16_t room) noexcept;

    private:
        const IScrollRuleProvider& _rules;
    };
}