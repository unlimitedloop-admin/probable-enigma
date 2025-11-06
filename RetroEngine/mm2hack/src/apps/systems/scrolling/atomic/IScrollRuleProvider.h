//==============================================================================
// 
//  Project: mm2hack
//  IScrollRuleProvider.h
// 
//  Provides the interface for scroll rule providers.
// 
//==============================================================================
#pragma once

#include <cstdint>
#include "ScrollTypes.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    // The interface for providing scrolling rules per page
    struct IScrollRuleProvider
    {
        virtual ~IScrollRuleProvider() = default;

        virtual ScrollKind RightType(std::size_t page_index) const = 0;
        virtual ScrollKind LeftType(std::size_t page_index) const = 0;
        virtual ScrollKind UpType(std::size_t page_index) const = 0;
        virtual ScrollKind DownType(std::size_t page_index) const = 0;

        virtual int16_t RightRoom(std::size_t page_index) const = 0;
        virtual int16_t LeftRoom(std::size_t page_index) const = 0;
        virtual int16_t UpRoom(std::size_t page_index) const = 0;
        virtual int16_t DownRoom(std::size_t page_index) const = 0;

        // room -> page index (-1 if not found)
        virtual int ToPageIndex(uint8_t room) const = 0;
    };
}