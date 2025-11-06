#include "pch.h"

#include "ScraperScrollRuleProvider.h"

#include "ScrollTypes.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    using Kind = ScrollKind;

    static Kind ToKind(int v) noexcept
    {
        switch (v)
        {
        case 0x01: return Kind::FreeHorizontal;
        case 0x02: return Kind::FixedPage;
        case 0x09: return Kind::FollowObject;
        case 0x0A: return Kind::Free8Way;
        default:   return Kind::None;
        }
    }

    ScrollKind ScraperScrollRuleProvider::RightType(std::size_t p) const
    {
        return ToKind(_scraper->getRightScrollType(static_cast<int>(p)));
    }

    ScrollKind ScraperScrollRuleProvider::LeftType(std::size_t p) const
    {
        return ToKind(_scraper->getLeftScrollType(static_cast<int>(p)));
    }

    ScrollKind ScraperScrollRuleProvider::UpType(std::size_t p) const
    {
        return ToKind(_scraper->getOverScrollType(static_cast<int>(p)));
    }

    ScrollKind ScraperScrollRuleProvider::DownType(std::size_t p) const
    {
        return ToKind(_scraper->getUnderScrollType(static_cast<int>(p)));
    }

    int16_t ScraperScrollRuleProvider::RightRoom(std::size_t p) const
    {
        return _scraper->getRightRoom(static_cast<int>(p));
    }

    int16_t ScraperScrollRuleProvider::LeftRoom(std::size_t p) const
    {
        return _scraper->getLeftRoom(static_cast<int>(p));
    }

    int16_t ScraperScrollRuleProvider::UpRoom(std::size_t p) const
    {
        return _scraper->getOverRoom(static_cast<int>(p));
    }

    int16_t ScraperScrollRuleProvider::DownRoom(std::size_t p) const
    {
        return _scraper->getUnderRoom(static_cast<int>(p));
    }

    int ScraperScrollRuleProvider::ToPageIndex(uint8_t room) const
    {
        return _scraper->getPageIndex(room);
    }
}