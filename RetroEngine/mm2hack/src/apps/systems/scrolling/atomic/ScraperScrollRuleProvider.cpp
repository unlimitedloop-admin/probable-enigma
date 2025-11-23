#include "pch.h"

#include "ScraperScrollRuleProvider.h"

#include "ScrollTypes.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    using Kind = ScrollKind;

    static Kind ToKindOpt(const std::optional<Kind>& v) noexcept
    {
        return v.has_value() ? v.value() : Kind::None;
    }

    ScrollKind ScraperScrollRuleProvider::RightType(std::size_t p) const
    {
        return ToKindOpt(_pageSrc ? _pageSrc->ScrollTypeRight(p) : std::optional<Kind>{});
    }

    ScrollKind ScraperScrollRuleProvider::LeftType(std::size_t p) const
    {
        return ToKindOpt(_pageSrc ? _pageSrc->ScrollTypeLeft(p) : std::optional<Kind>{});
    }

    ScrollKind ScraperScrollRuleProvider::UpType(std::size_t p) const
    {
        return ToKindOpt(_pageSrc ? _pageSrc->ScrollTypeUp(p) : std::optional<Kind>{});
    }

    ScrollKind ScraperScrollRuleProvider::DownType(std::size_t p) const
    {
        return ToKindOpt(_pageSrc ? _pageSrc->ScrollTypeDown(p) : std::optional<Kind>{});
    }

    int16_t ScraperScrollRuleProvider::RightRoom(std::size_t p) const
    {
        if (!_pageSrc) return static_cast<int16_t>(-1);
        const auto opt = _pageSrc->NeighborRight(p);
        return opt ? static_cast<int16_t>(*opt) : static_cast<int16_t>(-1);
    }

    int16_t ScraperScrollRuleProvider::LeftRoom(std::size_t p) const
    {
        if (!_pageSrc) return static_cast<int16_t>(-1);
        const auto opt = _pageSrc->NeighborLeft(p);
        return opt ? static_cast<int16_t>(*opt) : static_cast<int16_t>(-1);
    }

    int16_t ScraperScrollRuleProvider::UpRoom(std::size_t p) const
    {
        if (!_pageSrc) return static_cast<int16_t>(-1);
        const auto opt = _pageSrc->NeighborUp(p);
        return opt ? static_cast<int16_t>(*opt) : static_cast<int16_t>(-1);
    }

    int16_t ScraperScrollRuleProvider::DownRoom(std::size_t p) const
    {
        if (!_pageSrc) return static_cast<int16_t>(-1);
        const auto opt = _pageSrc->NeighborDown(p);
        return opt ? static_cast<int16_t>(*opt) : static_cast<int16_t>(-1);
    }

    int ScraperScrollRuleProvider::ToPageIndex(uint8_t room) const
    {
        if (!_pageSrc) return -1;
        const auto opt = _pageSrc->RoomToPageIndex(room);
        return opt ? static_cast<int>(*opt) : -1;
    }
}