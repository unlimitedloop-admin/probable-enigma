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
#include <unordered_map>
#include <utility>
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/resources/bg/IMapPageSource.h"
#include "ScrollTypes.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    using foundation::math::Vec2;

    // A thin wrapper around AddressScraper to provide scroll rules
    class ScraperScrollRuleProvider final : public IScrollRuleProvider
    {
        using IMapPageSource = apps::resources::bg::IMapPageSource;

    public:
        explicit ScraperScrollRuleProvider(std::shared_ptr<IMapPageSource> pageSrc)
            : _pageSrc(std::move(pageSrc))
        {
        }

        // Indirect calls to IMapPageSource
        ScrollKind RightType(std::size_t p) const override;
        // Indirect calls to IMapPageSource
        ScrollKind LeftType(std::size_t p) const override;
        // Indirect calls to IMapPageSource
        ScrollKind UpType(std::size_t p) const override;
        // Indirect calls to IMapPageSource
        ScrollKind DownType(std::size_t p) const override;

        // Indirect calls to IMapPageSource
        int16_t RightRoom(std::size_t p) const override;
        // Indirect calls to IMapPageSource
        int16_t LeftRoom(std::size_t p) const override;
        // Indirect calls to IMapPageSource
        int16_t UpRoom(std::size_t p) const override;
        // Indirect calls to IMapPageSource
        int16_t DownRoom(std::size_t p) const override;

        // Indirect calls to IMapPageSource
        int ToPageIndex(uint8_t room) const override;

        Vec2 PageOriginPx(std::size_t page_index, int page_w, int page_h) const override;

    private:
        void ensureGridCacheBuilt_() const;
        void tryAssignNeighbor_(std::unordered_map<std::size_t, std::pair<int, int>>& grid, std::size_t from, std::size_t to, int dx, int dy) const;

    private:
        const std::wstring kClassName{ L"ScraperScrollRuleProvider" };
        std::shared_ptr<IMapPageSource> _pageSrc;   // IMapPageSource instance

        // page_index -> (gridX, gridY)
        mutable bool _gridBuilt{ false };
        mutable std::unordered_map<std::size_t, std::pair<int, int>> _grid;
    };
}