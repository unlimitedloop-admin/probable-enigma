//==============================================================================
// 
//  Project: mm2hack
//  DemoStage1Phase.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include "DemoStage1.h"

#include <memory>
#include "apps/graphics/bg/AddressScraper.h"
#include "apps/mod/CoordinateTypes.h"

namespace mm2hack::apps::scenes
{
    namespace DemoStage1_
    {
        // Main phase - action stage scene
        class MainPhase : public IDemoStage1Phase
        {
            using AddressScraper = graphics::bg::AddressScraper;

        public:
            explicit MainPhase(DemoStage1& owner) : owner(owner) {}
            void Update() override;
            void RenderWorld() override;
            void RenderOverlay() override;
            DemoStage1PhaseId Id() const noexcept override;
            void SetAddressScraper(std::unique_ptr<AddressScraper> scraper) noexcept override;

        private:
            struct PageScroll
            {
                enum class Dir { None, Left, Right, Up, Down } dir{ Dir::None };
                bool   active{ false };
                double progress{ 0.0 };     // px 進捗（0..pageW/H）
                double speed{ 4.0 };        // 1フレームの移動px（調整用）
                std::size_t fromIndex{ 0 };
                std::size_t toIndex{ 0 };
            } _pg;

            void HV8WayScrollingUpdate_(const mod::Vec2 pos) noexcept;
            void HV8WayScrollingRender_() noexcept;

        private:
            DemoStage1& owner;
            mod::Point2 _objectPos{ 128.0, 120.0 };
            double _camX{ 0.0 };
            double _camY{ 0.0 };
            std::size_t _pageIndex{ 0 };
            int _tilePx{ 16 };

            std::unique_ptr<AddressScraper> _scraper;
        };
    }
}