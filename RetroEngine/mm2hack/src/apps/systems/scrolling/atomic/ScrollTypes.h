//==============================================================================
// 
//  Project: mm2hack
//  ScrollTypes.h
// 
//  Optimized scrolling type definitions.
// 
//==============================================================================
#pragma once

namespace mm2hack::apps::systems::scrolling::atomic
{
    enum class ScrollKind : int
    {
        None = 0x00,
        FreeHorizontal = 0x01,  // Side free scroll
        FixedPage = 0x02,       // Fixed page (animation)
        FollowObject = 0x09,    // Object follow
        Free8Way = 0x0A         // 8-direction scroll
    };

    struct PageScroll
    {
        // Direction of scroll
        enum class Dir { None, Right, Left, Down, Up };

        bool  active{ false };
        Dir   dir{ Dir::None };
        double progress{ 0.0 }; // px progress
        double speed{ 4.0 };    // px/frame (adjust as needed)

        std::size_t from_index{ 0 };
        std::size_t to_index{ 0 };
    };

    // Check if the scroll kind allows free movement
    inline constexpr bool IsAllowedFree(ScrollKind k) noexcept
    {
        const int v = static_cast<int>(k);
        return (v == 0x01) || (v == 0x09) || (v == 0x0A);
    }
}