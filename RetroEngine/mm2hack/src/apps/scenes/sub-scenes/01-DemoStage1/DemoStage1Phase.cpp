#include "pch.h"

#include "DemoStage1Phase.h"

#include <cstdint>
#include "apps/deal/GameContext.h"
#include "apps/graphics/bg/AddressScraper.h"
#include "apps/mod/CoordinateTypes.h"
#include "DemoStage1.h"
#include "input/Jpbtn.h"

namespace mm2hack::apps::scenes
{
    namespace DemoStage1_
    {
        //============================================================================== 
        //
        //  MainPhase
        //
        //==============================================================================
        void MainPhase::Update()
        {
            if (!owner.Fader().InputEnabled()) return;

            // Simple object movement with arrow keys
            using namespace mod;
            Vec2 delta{ 0, 0 };

            auto& input = owner.Input();
            if (input->IsPressed(JPBTN::UP))    delta.y -= 1;
            if (input->IsPressed(JPBTN::DOWN))  delta.y += 1;
            if (input->IsPressed(JPBTN::LEFT))  delta.x -= 1;
            if (input->IsPressed(JPBTN::RIGHT)) delta.x += 1;

            HV8WayScrollingUpdate_(delta);
        }

        void MainPhase::RenderWorld()
        {
            // Render the world elements
            using namespace config;
            using namespace deal;
            auto& bgTileManager = GameContext::GetInstance().GetResourceManager().GetBGTileManager();

            // Draw the graph from the resource manager.
            HV8WayScrollingRender_();

            ::DxLib::DrawLine(static_cast<int>(_objectPos.x), 0, static_cast<int>(_objectPos.x), SystemConfig::kScreenHeight, 0xFFFF0000, 2);   // Vertical line
            ::DxLib::DrawLine(0, static_cast<int>(_objectPos.y), SystemConfig::kScreenWidth,  static_cast<int>(_objectPos.y), 0xFFFF0000, 2);   // Horizontal line
        }

        void MainPhase::RenderOverlay()
        {
            // Render the overlay elements for the demo stage here.
        }

        DemoStage1PhaseId MainPhase::Id() const noexcept
        {
            return DemoStage1PhaseId::Main;
        }

        void MainPhase::SetAddressScraper(std::unique_ptr<AddressScraper> scraper) noexcept
        {
            _scraper = std::move(scraper);
        }

        void MainPhase::HV8WayScrollingUpdate_(const mod::Vec2 pos) noexcept
        {
            constexpr double kCenterX = 128.0; // 256/2
            constexpr double kCenterY = 120.0; // 240/2
            const int pageW = 16 * _tilePx; // 256
            const int pageH = 15 * _tilePx; // 240
            const int viewW = config::SystemConfig::kScreenWidth;   // 256
            const int viewH = config::SystemConfig::kScreenHeight;  // 240

            auto scrollTypeR = [&](std::size_t p) { return _scraper->getRightScrollType(static_cast<int>(p)); };
            auto scrollTypeL = [&](std::size_t p) { return _scraper->getLeftScrollType(static_cast<int>(p)); };
            auto scrollTypeU = [&](std::size_t p) { return _scraper->getOverScrollType(static_cast<int>(p)); };
            auto scrollTypeD = [&](std::size_t p) { return _scraper->getUnderScrollType(static_cast<int>(p)); };
            auto hasRight = [&](std::size_t p) { return _scraper->getRightRoom(static_cast<int>(p)) >= 0; };
            auto hasLeft = [&](std::size_t p) { return _scraper->getLeftRoom(static_cast<int>(p)) >= 0; };
            auto hasUp = [&](std::size_t p) { return _scraper->getOverRoom(static_cast<int>(p)) >= 0; };
            auto hasDown = [&](std::size_t p) { return _scraper->getUnderRoom(static_cast<int>(p)) >= 0; };
            auto toIndex = [&](int16_t room) { return _scraper->getPageIndex(static_cast<std::uint8_t>(room)); };

            auto isScrollableNibble = [](int t)->bool { return t == 0x01 || t == 0x09 || t == 0x0A; }; // ★テスト中は 2 を不許可

            // --- X軸：まず十字線を中心へ“食わせる”、残りをカメラへ ---
            if (pos.x != 0.0)
            {
                double remain = pos.x;

                // 1) 十字線を中心へ寄せる（ワープ防止）
                if (remain > 0.0 && _objectPos.x < kCenterX)
                {
                    const double need = kCenterX - _objectPos.x;
                    const double step = std::min(remain, need);
                    _objectPos.x += step;
                    remain -= step;
                }
                else if (remain < 0.0 && _objectPos.x > kCenterX)
                {
                    const double need = _objectPos.x - kCenterX;
                    const double step = std::min(-remain, need);
                    _objectPos.x -= step;
                    remain += step;
                }

                // 2) 残量をカメラに適用／境界越え時だけ隣室の可否を参照
                if (!mod::NearlyZero(remain))
                {
                    double newCamX = _camX + remain;

                    if (newCamX >= 0.0 && newCamX < pageW)
                    {
                        _camX = newCamX;
                        _objectPos.x = kCenterX;
                    }
                    else if (newCamX < 0.0)
                    {
                        // 左境界越え
                        const bool canCross =
                            hasLeft(_pageIndex) &&
                            isScrollableNibble(scrollTypeL(_pageIndex));
                        if (canCross)
                        {
                            const int16_t room = _scraper->getLeftRoom(static_cast<int>(_pageIndex));
                            _pageIndex = static_cast<std::size_t>(toIndex(room));
                            _camX = newCamX + pageW;   // 折り返し
                            _objectPos.x = kCenterX;
                        }
                        else
                        {
                            // 画面固定→十字線だけ動かす
                            _objectPos.x = std::clamp(_objectPos.x + remain, 0.0, static_cast<double>(viewW - 1));
                        }
                    }
                    else
                    { // newCamX >= pageW
                             // 右境界越え
                        const bool canCross =
                            hasRight(_pageIndex) &&
                            isScrollableNibble(scrollTypeR(_pageIndex));
                        if (canCross)
                        {
                            const int16_t room = _scraper->getRightRoom(static_cast<int>(_pageIndex));
                            _pageIndex = static_cast<std::size_t>(toIndex(room));
                            _camX = newCamX - pageW;
                            _objectPos.x = kCenterX;
                        }
                        else
                        {
                            _objectPos.x = std::clamp(_objectPos.x + remain, 0.0, static_cast<double>(viewW - 1));
                        }
                    }
                }
            }

            // --- Y軸：Xと同じロジック ---
            if (pos.y != 0.0)
            {
                double remain = pos.y;

                if (remain > 0.0 && _objectPos.y < kCenterY)
                {
                    const double need = kCenterY - _objectPos.y;
                    const double step = std::min(remain, need);
                    _objectPos.y += step;
                    remain -= step;
                }
                else if (remain < 0.0 && _objectPos.y > kCenterY)
                {
                    const double need = _objectPos.y - kCenterY;
                    const double step = std::min(-remain, need);
                    _objectPos.y -= step;
                    remain += step;
                }

                if (!mod::NearlyZero(remain))
                {
                    double newCamY = _camY + remain;

                    if (newCamY >= 0.0 && newCamY < pageH)
                    {
                        _camY = newCamY;
                        _objectPos.y = kCenterY;
                    }
                    else if (newCamY < 0.0)
                    {
                        const bool canCross =
                            hasUp(_pageIndex) &&
                            isScrollableNibble(scrollTypeU(_pageIndex));
                        if (canCross)
                        {
                            const int16_t room = _scraper->getOverRoom(static_cast<int>(_pageIndex));
                            _pageIndex = static_cast<std::size_t>(toIndex(room));
                            _camY = newCamY + pageH;
                            _objectPos.y = kCenterY;
                        }
                        else
                        {
                            _objectPos.y = std::clamp(_objectPos.y + remain, 0.0, static_cast<double>(viewH - 1));
                        }
                    }
                    else
                    { // newCamY >= pageH
                        const bool canCross =
                            hasDown(_pageIndex) &&
                            isScrollableNibble(scrollTypeD(_pageIndex));
                        if (canCross)
                        {
                            const int16_t room = _scraper->getUnderRoom(static_cast<int>(_pageIndex));
                            _pageIndex = static_cast<std::size_t>(toIndex(room));
                            _camY = newCamY - pageH;
                            _objectPos.y = kCenterY;
                        }
                        else
                        {
                            _objectPos.y = std::clamp(_objectPos.y + remain, 0.0, static_cast<double>(viewH - 1));
                        }
                    }
                }
            }
        }

        void MainPhase::HV8WayScrollingRender_() noexcept
        {
            using namespace deal;
            using namespace config;
            auto& bg = GameContext::GetInstance().GetResourceManager().GetBGTileManager();
            using Scraper = graphics::bg::AddressScraper;

            const int pageW = _tilePx * 16; // 256
            const int pageH = _tilePx * 15; // 240

            // 現在ページ内のオフセット（0..pageW/H）
            // 画面に対しては「-オフセット」で描画
            const int ox = -static_cast<int>(_camX);
            const int oy = -static_cast<int>(_camY);

            // 1) 現在ページ
            bg.LoadMapBinary(owner.GetMapBinaryPath(), static_cast<int>(_pageIndex * 0x100 + 0x10));
            bg.DrawMapByName(owner.GetMapName(), SystemConfig::kTileSizeWidth, SystemConfig::kTileSizeHeight, ox, oy);

            // 2) 必要に応じて隣接ページを重ねる
            // 右
            if (ox + pageW < SystemConfig::kScreenWidth)
            {
                const int16_t room = _scraper->getRightRoom(static_cast<int>(_pageIndex));
                if (room >= 0)
                {
                    const auto idx = _scraper->getPageIndex(static_cast<uint8_t>(room));
                    if (idx >= 0)
                    {
                        bg.LoadMapBinary(owner.GetMapBinaryPath(), static_cast<int>(idx * 0x100 + 0x10));
                        bg.DrawMapByName(owner.GetMapName(), SystemConfig::kTileSizeWidth, SystemConfig::kTileSizeHeight,
                            ox + pageW, oy);
                    }
                }
            }
            // 下
            if (oy + pageH < SystemConfig::kScreenHeight)
            {
                const int16_t room = _scraper->getUnderRoom(static_cast<int>(_pageIndex));
                if (room >= 0)
                {
                    const auto idx = _scraper->getPageIndex(static_cast<uint8_t>(room));
                    if (idx >= 0)
                    {
                        bg.LoadMapBinary(owner.GetMapBinaryPath(), static_cast<int>(idx * 0x100 + 0x10));
                        bg.DrawMapByName(owner.GetMapName(), SystemConfig::kTileSizeWidth, SystemConfig::kTileSizeHeight,
                            ox, oy + pageH);
                    }
                }
            }
            // 右下
            if (ox + pageW < SystemConfig::kScreenWidth && oy + pageH < SystemConfig::kScreenHeight)
            {
                const int16_t rRoom = _scraper->getRightRoom(static_cast<int>(_pageIndex));
                if (rRoom >= 0)
                {
                    const auto rIdx = _scraper->getPageIndex(static_cast<uint8_t>(rRoom));
                    if (rIdx >= 0)
                    {
                        const int16_t rdRoom = _scraper->getUnderRoom(static_cast<int>(rIdx));
                        if (rdRoom >= 0)
                        {
                            const auto rdIdx = _scraper->getPageIndex(static_cast<uint8_t>(rdRoom));
                            if (rdIdx >= 0)
                            {
                                bg.LoadMapBinary(owner.GetMapBinaryPath(), static_cast<int>(rdIdx * 0x100 + 0x10));
                                bg.DrawMapByName(owner.GetMapName(), SystemConfig::kTileSizeWidth, SystemConfig::kTileSizeHeight,
                                    ox + pageW, oy + pageH);
                            }
                        }
                    }
                }
            }
            // 左／上／左上も必要なら同様に（_camX/_camY が負寄りに振れるケースに備えて）
            if (ox > 0)
            {
                const int16_t room = _scraper->getLeftRoom(static_cast<int>(_pageIndex));
                if (room >= 0)
                {
                    const auto idx = _scraper->getPageIndex(static_cast<uint8_t>(room));
                    if (idx >= 0)
                    {
                        bg.LoadMapBinary(owner.GetMapBinaryPath(), static_cast<int>(idx * 0x100 + 0x10));
                        bg.DrawMapByName(owner.GetMapName(), SystemConfig::kTileSizeWidth, SystemConfig::kTileSizeHeight,
                            ox - pageW, oy);
                    }
                }
            }
            if (oy > 0)
            {
                const int16_t room = _scraper->getOverRoom(static_cast<int>(_pageIndex));
                if (room >= 0)
                {
                    const auto idx = _scraper->getPageIndex(static_cast<uint8_t>(room));
                    if (idx >= 0)
                    {
                        bg.LoadMapBinary(owner.GetMapBinaryPath(), static_cast<int>(idx * 0x100 + 0x10));
                        bg.DrawMapByName(owner.GetMapName(), SystemConfig::kTileSizeWidth, SystemConfig::kTileSizeHeight,
                            ox, oy - pageH);
                    }
                }
            }
            if (ox > 0 && oy > 0)
            {
                const int16_t lRoom = _scraper->getLeftRoom(static_cast<int>(_pageIndex));
                if (lRoom >= 0)
                {
                    const auto lIdx = _scraper->getPageIndex(static_cast<uint8_t>(lRoom));
                    if (lIdx >= 0)
                    {
                        const int16_t luRoom = _scraper->getOverRoom(static_cast<int>(lIdx));
                        if (luRoom >= 0)
                        {
                            const auto luIdx = _scraper->getPageIndex(static_cast<uint8_t>(luRoom));
                            if (luIdx >= 0)
                            {
                                bg.LoadMapBinary(owner.GetMapBinaryPath(), static_cast<int>(luIdx * 0x100 + 0x10));
                                bg.DrawMapByName(owner.GetMapName(), SystemConfig::kTileSizeWidth, SystemConfig::kTileSizeHeight,
                                    ox - pageW, oy - pageH);
                            }
                        }
                    }
                }
            }
        }
    }
}