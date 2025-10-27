//==============================================================================
// 
//  Project: mm2hack
//  IScrollRuleProvider.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include <cstdint>
#include "ScrollTypes.h"

namespace mm2hack::apps::algorithm::scrolling::atomic
{
    // スクロール可否と隣接ページを提供するインタフェース
    struct IScrollRuleProvider
    {
        virtual ~IScrollRuleProvider() = default;

        virtual ScrollKind RightType(std::size_t page_index) const = 0;
        virtual ScrollKind LeftType(std::size_t page_index) const = 0;
        virtual ScrollKind UpType(std::size_t page_index) const = 0;
        virtual ScrollKind DownType(std::size_t page_index) const = 0;

        virtual int16_t RightRoom(std::size_t page_index) const = 0;
        virtual int16_t LeftRoom(
            std::size_t page_index) const = 0;
        virtual int16_t UpRoom(
            std::size_t page_index) const = 0;
        virtual int16_t DownRoom(
            std::size_t page_index) const = 0;

        // room(0..255) → page index(>=0) / 無効なら -1
        virtual int ToPageIndex(uint8_t room) const = 0;
    };
}