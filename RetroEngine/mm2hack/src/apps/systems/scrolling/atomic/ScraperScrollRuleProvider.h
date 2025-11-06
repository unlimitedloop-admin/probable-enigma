//==============================================================================
// 
//  Project: mm2hack
//  ScraperScrollRuleProvider.h
// 
//  Provides scroll rules using AddressScraper.
// 
//==============================================================================
#pragma once

#include "IScrollRuleProvider.h"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include "apps/resources/bg/AddressScraper.h"
#include "ScrollTypes.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    // A thin wrapper around AddressScraper to provide scroll rules
    class ScraperScrollRuleProvider final : public IScrollRuleProvider
    {
        using AddressScraper = apps::resources::bg::AddressScraper;

    public:
        explicit ScraperScrollRuleProvider(std::shared_ptr<AddressScraper> scraper)
            : _scraper(std::move(scraper))
        {
        }

        // Indirect calls to AddressScraper
        ScrollKind RightType(std::size_t p) const override;
        // Indirect calls to AddressScraper
        ScrollKind LeftType(std::size_t p) const override;
        // Indirect calls to AddressScraper
        ScrollKind UpType(std::size_t p) const override;
        // Indirect calls to AddressScraper
        ScrollKind DownType(std::size_t p) const override;

        // Indirect calls to AddressScraper
        int16_t RightRoom(std::size_t p) const override;
        // Indirect calls to AddressScraper
        int16_t LeftRoom(std::size_t p) const override;
        // Indirect calls to AddressScraper
        int16_t UpRoom(std::size_t p) const override;
        // Indirect calls to AddressScraper
        int16_t DownRoom(std::size_t p) const override;

        // Indirect calls to AddressScraper
        int ToPageIndex(uint8_t room) const override;

    private:
        const std::wstring kClassName = L"ScraperScrollRuleProvider";

        std::shared_ptr<AddressScraper> _scraper;   // AddressScraper instance
    };
}