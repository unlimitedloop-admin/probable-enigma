//==============================================================================
// 
//  Project: mm2hack
//  ScrollTypes.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

namespace mm2hack::apps::algorithm::scrolling::atomic
{
    enum class ScrollKind : int
    {
        None = 0x00,
        FreeHorizontal = 0x01,  // サイドフリースクロール
        FixedPage = 0x02,       // 固定ページ（アニメ）
        FollowObject = 0x09,    // オブジェクト追従
        Free8Way = 0x0A         // 8方向スクロール
    };

    struct PageScroll
    {
        // Direction of scroll
        enum class Dir { None, Right, Left, Down, Up };

        bool  active{ false };
        Dir   dir{ Dir::None };
        double progress{ 0.0 }; // px 進捗
        double speed{ 4.0 };    // px/frame（必要に応じて調整）

        std::size_t from_index{ 0 };
        std::size_t to_index{ 0 };
    };

    inline constexpr bool IsAllowedFree(ScrollKind k) noexcept
    {
        const int v = static_cast<int>(k);
        return (v == 0x01) || (v == 0x09) || (v == 0x0A);
    }
}