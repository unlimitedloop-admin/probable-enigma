//==============================================================================
// 
//  Project: mm2hack
//  RockBusterOffsetTable.h
// 
//  Defines offset rules for the Rock Buster ability based on avatar base poses.
// 
//==============================================================================
#pragma once

#include <iterator>
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/world/entity/avatar/AvatarStatus.h"

namespace mm2hack::apps::world::entity::avatar::abilities
{
    using foundation::math::Vec2;

    // Offset rule structure definition
    struct OffsetRule final
    {
        int from{};     // Inclusive start of base pose range
        int to{};       // Inclusive end of base pose range
        Vec2 offset{};  // Offset to apply [x, y]
    };

    static constexpr OffsetRule kRockBusterOffsetRight[] =
    {
        { 1, 3,  { 28.0, 10.0 } },
        { 4, 5,  { 24.0, 11.0 } },
        { 6, 6,  { 24.0,  9.0 } },
        { 7, 7,  { 25.0,  9.0 } },
        { 20, 23, { 24.0, 10.0 } },
    };

    static constexpr OffsetRule kRockBusterOffsetLeft[] =
    {
        { 1, 3,  { -4.0, 10.0 } },
        { 4, 5,  {  0.0, 11.0 } },
        { 6, 6,  {  0.0,  9.0 } },
        { 7, 7,  { -1.0,  9.0 } },
        { 20, 23, {  0.0, 10.0 } },
    };

    // Find offset by base pose from given rules
    static constexpr Vec2 FindOffset(int base_pose, const OffsetRule* rules, std::size_t count) noexcept
    {
        for (std::size_t i = 0; i < count; ++i)
        {
            const auto& r = rules[i];
            if (base_pose >= r.from && base_pose <= r.to)
            {
                return r.offset;
            }
        }
        return Vec2{ 0.0, 0.0 };
    }

    // Find Rock Buster offset by base pose and facing direction
    static constexpr Vec2 FindRockBusterOffsetByBasePose(int base_pose, AvatarDirection facing) noexcept
    {
        if (facing == AvatarDirection::Left)
        {
            return FindOffset(base_pose, kRockBusterOffsetLeft, std::size(kRockBusterOffsetLeft));
        }
        return FindOffset(base_pose, kRockBusterOffsetRight, std::size(kRockBusterOffsetRight));
    }
}