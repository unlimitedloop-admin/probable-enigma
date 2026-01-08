//==============================================================================
// 
//  Project: mm2hack
//  FreeScrollDriver.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include "apps/foundation/math/CoordinateTypes.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    struct Camera;
    class IScrollRuleProvider;
    class ScrollNeighborResolver;

    // Free-scroll management that handles both axes
    class FreeScrollDriver final
    {
        using Vec2 = foundation::math::Vec2;

    public:
        struct Params
        {
            int tile_px{};
            int tileX{};
            int tileY{};
        };

        FreeScrollDriver(const IScrollRuleProvider& rules, const ScrollNeighborResolver& resolver, Params params) noexcept
            : _rules(rules), _resolver(resolver), _params(params)
        {
        }

        void Update(
            const Vec2& input_delta, const Vec2& object_pos, const Vec2& target_pos,
            std::size_t& page_index, Vec2& view_world, Camera& cam) const;

    private:
        // Axis update helpers
        void updateAxisX_(
            const double remain, const Vec2& object_pos, const Vec2& target_pos,
            std::size_t& page_index, Vec2& view_world, Camera& cam) const;
        void updateAxisY_(
            const double remain, const Vec2& object_pos, const Vec2& target_pos,
            std::size_t& page_index, Vec2& view_world, Camera& cam) const;

        void normalizeViewWorldToPage_(std::size_t& page_index, Vec2& view_world, Camera& cam) const;

    private:
        const IScrollRuleProvider& _rules;
        const ScrollNeighborResolver& _resolver;
        Params _params{};
    };
}