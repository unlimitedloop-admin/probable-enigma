//==============================================================================
// 
//  Project: mm2hack
//  ScraperScrollRuleProvider.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include "IScrollRuleProvider.h"

#include <cstdint>
#include <memory>
#include <utility>
#include "apps/graphics/bg/AddressScraper.h"
#include "ScrollTypes.h"

namespace mm2hack::apps::algorithm::scrolling::atomic
{
    // AddressScraper の薄いアダプタ
    class ScraperScrollRuleProvider final : public IScrollRuleProvider
    {
        using AddressScraper = graphics::bg::AddressScraper;

    public:
        explicit ScraperScrollRuleProvider(std::shared_ptr<AddressScraper> scraper)
            : _scraper(std::move(scraper))
        {
        }

        ScrollKind RightType(std::size_t p) const override;
        ScrollKind LeftType(std::size_t p) const override;
        ScrollKind UpType(std::size_t p) const override;
        ScrollKind DownType(std::size_t p) const override;

        int16_t RightRoom(std::size_t p) const override;
        int16_t LeftRoom(std::size_t p) const override;
        int16_t UpRoom(std::size_t p) const override;
        int16_t DownRoom(std::size_t p) const override;

        int ToPageIndex(uint8_t room) const override;

    private:
        std::shared_ptr<AddressScraper> _scraper;
    };
}