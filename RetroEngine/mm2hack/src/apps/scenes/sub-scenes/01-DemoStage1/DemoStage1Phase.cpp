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
            std::wstring s = std::wstring(L"PageIndex = ") + std::to_wstring(_pageIndex);
            ::DxLib::DrawString(8, 8, s.c_str(), 0xFFFFFF00);
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

            // 事前準備（Update内のラムダはそのまま使ってOK）
            auto rightType = [&](std::size_t p) { return _scraper->getRightScrollType(static_cast<int>(p)); };
            auto leftType = [&](std::size_t p) { return _scraper->getLeftScrollType(static_cast<int>(p)); };
            auto upType = [&](std::size_t p) { return _scraper->getOverScrollType(static_cast<int>(p)); };
            auto downType = [&](std::size_t p) { return _scraper->getUnderScrollType(static_cast<int>(p)); };

            auto rightRoom = [&](std::size_t p) { return _scraper->getRightRoom(static_cast<int>(p)); };
            auto leftRoom = [&](std::size_t p) { return _scraper->getLeftRoom(static_cast<int>(p)); };
            auto upRoom = [&](std::size_t p) { return _scraper->getOverRoom(static_cast<int>(p)); };
            auto downRoom = [&](std::size_t p) { return _scraper->getUnderRoom(static_cast<int>(p)); };
            auto toIndex = [&](int16_t room) { return _scraper->getPageIndex(static_cast<std::uint8_t>(room)); };

            auto isAllowedFree = [](int t)->bool { return t == 0x01 || t == 0x09 || t == 0x0A; };   // 自由スクロール
            auto isFixedPage = [](int t)->bool { return t == 0x02; };                               // 固定ページ（アニメ）

            // === 固定ページスクロール中：進捗だけ更新 ===
            if (_pg.active)
            {
                const int pageW = 16 * _tilePx; // 256
                const int pageH = 15 * _tilePx; // 240
                const int maxX = config::SystemConfig::kScreenWidth - 1; // 255
                const int maxY = config::SystemConfig::kScreenHeight - 1; // 239

                // 進捗
                _pg.progress += _pg.speed;
                // 0..1 に正規化
                const double tX = std::clamp(_pg.progress / static_cast<double>(pageW), 0.0, 1.0);
                const double tY = std::clamp(_pg.progress / static_cast<double>(pageH), 0.0, 1.0);

                // ★ プレイヤー座標を「端 → 対岸」に補間
                switch (_pg.dir)
                {
                case PageScroll::Dir::Right:
                    _objectPos.x = (1.0 - tX) * maxX;
                    _objectPos.y = _objectPos.y;
                    break;           // 255→0

                case PageScroll::Dir::Left:
                    _objectPos.x = tX * maxX;
                    _objectPos.y = _objectPos.y;
                    break;           // 0→255

                case PageScroll::Dir::Down:
                    _objectPos.y = (1.0 - tY) * maxY;
                    _objectPos.x = _objectPos.x;
                    break;           // 239→0

                case PageScroll::Dir::Up:
                    _objectPos.y = tY * maxY;
                    _objectPos.x = _objectPos.x;
                    break;           // 0→239

                default:
                    break;
                }

                // 完了チェック
                const double need = (_pg.dir == PageScroll::Dir::Left || _pg.dir == PageScroll::Dir::Right) ? pageW : pageH;
                if (_pg.progress >= need)
                {
                    _pageIndex = _pg.toIndex;
                    // 対岸で確定
                    switch (_pg.dir)
                    {
                    case PageScroll::Dir::Right: _objectPos.x = 0;                break;
                    case PageScroll::Dir::Left:  _objectPos.x = maxX;             break;
                    case PageScroll::Dir::Down:  _objectPos.y = 0;                break;
                    case PageScroll::Dir::Up:    _objectPos.y = maxY;             break;
                    default: break;
                    }
                    _pg = PageScroll{}; // reset
                    _camX = 0.0; _camY = 0.0;
                }
                return; // アニメ中は通常更新しない
            }

            // === フリースクロール処理 ===
            // ---------- X軸 ----------
            if (pos.x != 0.0)
            {
                double remain = pos.x;

                // 1) 十字線→中央に寄せる（ワープ防止）
                if (remain > 0.0 && _objectPos.x < kCenterX)
                {
                    const double take = std::min(remain, kCenterX - _objectPos.x);
                    _objectPos.x += take; remain -= take;
                }
                else if (remain < 0.0 && _objectPos.x > kCenterX)
                {
                    const double take = std::min(-remain, _objectPos.x - kCenterX);
                    _objectPos.x -= take; remain += take;
                }

                // 2) 残量を“方向性”で分岐させてカメラへ
                if (!mod::NearlyZero(remain))
                {
                    // 正方向（右）に押している
                    if (remain > 0.0)
                    {
                        const double tentative = _camX + remain;

                        if (tentative <= 0.0)
                        {
                            // 0 へ向かう戻りは常にOK
                            _camX = tentative;
                            _objectPos.x = kCenterX;
                        }
                        else
                        {
                            const int t = rightType(_pageIndex);
                            const int16_t rr = rightRoom(_pageIndex);
                            const int rIdx = rr >= 0 ? toIndex(rr) : -1;

                            if (isAllowedFree(t) && rIdx >= 0)
                            {
                                // 自由スクロール
                                _camX = tentative; _objectPos.x = kCenterX;
                                while (_camX >= pageW) { _pageIndex = static_cast<std::size_t>(rIdx); _camX -= pageW; }
                            }
                            else if (isFixedPage(t) && rIdx >= 0)
                            {
                                // ★ まずプレイヤーを右端(255)まで動かす
                                const int maxX = viewW - 1;
                                double need = static_cast<double>(maxX) - _objectPos.x;
                                if (need > 0.0)
                                {
                                    const double step = std::min(remain, need);   // remain は正
                                    _objectPos.x += step;
                                    remain -= step;
                                }
                                // 端に到達し、まだ押しているなら開始
                                if (_objectPos.x >= maxX && remain > 0.0)
                                {
                                    _pg.active = true; _pg.dir = PageScroll::Dir::Right;
                                    _pg.progress = 0.0; _pg.fromIndex = _pageIndex; _pg.toIndex = static_cast<std::size_t>(rIdx);
                                    _camX = 0.0; // 固定演出中はカメラを動かさない
                                }
                                // 端に到達していなければここで終了（このフレームは開始しない）
                            }
                            else
                            {
                                // 不許可：線だけ動かす
                                _objectPos.x = std::clamp(_objectPos.x + remain, 0.0, static_cast<double>(viewW - 1));
                            }
                        }
                    }
                    // 負方向（左）に押している
                    else /* remain < 0.0 */
                    {
                        const double tentative = _camX + remain;

                        if (tentative >= 0.0)
                        {
                            // 0 へ向かう戻りは常にOK
                            _camX = tentative;
                            _objectPos.x = kCenterX;
                        }
                        else
                        {
                            const int t = leftType(_pageIndex);
                            const int16_t lr = leftRoom(_pageIndex);
                            const int lIdx = lr >= 0 ? toIndex(lr) : -1;

                            if (isAllowedFree(t) && lIdx >= 0)
                            {
                                _camX = tentative; _objectPos.x = kCenterX;
                                while (_camX < 0.0) { _pageIndex = static_cast<std::size_t>(lIdx); _camX += pageW; }
                            }
                            else if (isFixedPage(t) && lIdx >= 0)
                            {
                                const int minX = 0;
                                double need = _objectPos.x - static_cast<double>(minX);
                                if (need > 0.0)
                                {
                                    const double step = std::min(-remain, need);  // remain は負
                                    _objectPos.x -= step;
                                    remain += step;
                                }
                                if (_objectPos.x <= minX && remain < 0.0)
                                {
                                    _pg.active = true; _pg.dir = PageScroll::Dir::Left;
                                    _pg.progress = 0.0; _pg.fromIndex = _pageIndex; _pg.toIndex = static_cast<std::size_t>(lIdx);
                                    _camX = 0.0;
                                }
                            }
                            else
                            {
                                _objectPos.x = std::clamp(_objectPos.x + remain, 0.0, static_cast<double>(viewW - 1));
                            }
                        }
                    }
                }
            }

            // ---------- Y軸 ----------
            if (pos.y != 0.0)
            {
                double remain = pos.y;

                if (remain > 0.0 && _objectPos.y < kCenterY)
                {
                    const double take = std::min(remain, kCenterY - _objectPos.y);
                    _objectPos.y += take; remain -= take;
                }
                else if (remain < 0.0 && _objectPos.y > kCenterY)
                {
                    const double take = std::min(-remain, _objectPos.y - kCenterY);
                    _objectPos.y -= take; remain += take;
                }

                if (!mod::NearlyZero(remain))
                {
                    if (remain > 0.0)
                    {
                        // 下へ
                        const double tentative = _camY + remain;

                        if (tentative <= 0.0)
                        {
                            _camY = tentative;
                            _objectPos.y = kCenterY;
                        }
                        else
                        {
                            const int t = downType(_pageIndex);
                            const int16_t dr = downRoom(_pageIndex);
                            const int dIdx = dr >= 0 ? toIndex(dr) : -1;

                            if (isAllowedFree(t) && dIdx >= 0)
                            {
                                _camY = tentative; _objectPos.y = kCenterY;
                                while (_camY >= pageH) { _pageIndex = static_cast<std::size_t>(dIdx); _camY -= pageH; }
                            }
                            else if (isFixedPage(t) && dIdx >= 0)
                            {
                                const int maxY = viewH - 1;
                                double need = static_cast<double>(maxY) - _objectPos.y;
                                if (need > 0.0)
                                {
                                    const double step = std::min(remain, need);   // remain は正
                                    _objectPos.y += step;
                                    remain -= step;
                                }
                                if (_objectPos.y >= maxY && remain > 0.0)
                                {
                                    _pg.active = true; _pg.dir = PageScroll::Dir::Down;
                                    _pg.progress = 0.0; _pg.fromIndex = _pageIndex; _pg.toIndex = static_cast<std::size_t>(dIdx);
                                    _camY = 0.0;
                                }
                            }
                            else
                            {
                                _objectPos.y = std::clamp(_objectPos.y + remain, 0.0, static_cast<double>(viewH - 1));
                            }
                        }
                    }
                    else
                    {
                        // remain < 0.0 上へ
                        const double tentative = _camY + remain;

                        if (tentative >= 0.0)
                        {
                            _camY = tentative;
                            _objectPos.y = kCenterY;
                        }
                        else
                        {
                            const int t = upType(_pageIndex);
                            const int16_t ur = upRoom(_pageIndex);
                            const int uIdx = ur >= 0 ? toIndex(ur) : -1;

                            if (isAllowedFree(t) && uIdx >= 0)
                            {
                                _camY = tentative; _objectPos.y = kCenterY;
                                while (_camY < 0.0) { _pageIndex = static_cast<std::size_t>(uIdx); _camY += pageH; }
                            }
                            else if (isFixedPage(t) && uIdx >= 0)
                            {
                                const int minY = 0;
                                double need = _objectPos.y - static_cast<double>(minY);
                                if (need > 0.0)
                                {
                                    const double step = std::min(-remain, need);  // remain は負
                                    _objectPos.y -= step;
                                    remain += step;
                                }
                                if (_objectPos.y <= minY && remain < 0.0)
                                {
                                    _pg.active = true; _pg.dir = PageScroll::Dir::Up;
                                    _pg.progress = 0.0; _pg.fromIndex = _pageIndex; _pg.toIndex = static_cast<std::size_t>(uIdx);
                                    _camY = 0.0;
                                }
                            }
                            else
                            {
                                _objectPos.y = std::clamp(_objectPos.y + remain, 0.0, static_cast<double>(viewH - 1));
                            }
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

            if (_pg.active)
            {
                // 固定ページスクロール演出中
                const int prog = static_cast<int>(_pg.progress);

                // 2ページ確実に描くため、DrawMapBinary→DrawMapByName を2回だけ
                auto drawPage = [&](std::size_t pageIndex, int dx, int dy)
                    {
                        bg.LoadMapBinary(owner.GetMapBinaryPath(), static_cast<int>(pageIndex * 0x100 + 0x10));
                        bg.DrawMapByName(owner.GetMapName(),
                            SystemConfig::kTileSizeWidth, SystemConfig::kTileSizeHeight,
                            dx, dy);
                    };

                switch (_pg.dir)
                {
                case PageScroll::Dir::Right:
                    // 現在ページ：左へ prog px
                    // 右ページ： (pageW - prog) から入ってくる
                    drawPage(_pg.fromIndex, -prog, 0);
                    drawPage(_pg.toIndex, pageW - prog, 0);
                    break;
                case PageScroll::Dir::Left:
                    drawPage(_pg.fromIndex, prog, 0);
                    drawPage(_pg.toIndex, -pageW + prog, 0);
                    break;
                case PageScroll::Dir::Down:
                    drawPage(_pg.fromIndex, 0, -prog);
                    drawPage(_pg.toIndex, 0, pageH - prog);
                    break;
                case PageScroll::Dir::Up:
                    drawPage(_pg.fromIndex, 0, prog);
                    drawPage(_pg.toIndex, 0, -pageH + prog);
                    break;
                default: break;
                }

                // 十字線（中央固定）
                ::DxLib::DrawLine(static_cast<int>(_objectPos.x), 0,
                    static_cast<int>(_objectPos.x), SystemConfig::kScreenHeight,
                    0xFFFF0000, 2);
                ::DxLib::DrawLine(0, static_cast<int>(_objectPos.y),
                    SystemConfig::kScreenWidth, static_cast<int>(_objectPos.y),
                    0xFFFF0000, 2);
                return;
            }

            // 現在ページ内のオフセット（0..pageW/H）
            // 画面に対しては「-オフセット」で描画
            const int ox = -static_cast<int>(_camX);
            const int oy = -static_cast<int>(_camY);

            // 1) 現在ページ
            bg.LoadMapBinary(owner.GetMapBinaryPath(), static_cast<int>(_pageIndex * 0x100 + 0x10));
            bg.DrawMapByName(owner.GetMapName(), SystemConfig::kTileSizeWidth, SystemConfig::kTileSizeHeight, ox, oy);

            // ★許可するスクロールタイプ (0x01 = サイドフリースクロール、0x09 = オブジェクト追従、0x0A = 8方向スクロール)
            const auto isAllowed = [](int t) { return t == 0x01 || t == 0x09 || t == 0x0A; };

            // 2) 必要に応じて隣接ページを重ねる
            // 右
            if (ox + pageW < SystemConfig::kScreenWidth)
            {
                const int t = _scraper->getRightScrollType(static_cast<int>(_pageIndex));
                const int16_t room = _scraper->getRightRoom(static_cast<int>(_pageIndex));
                const int idx = _scraper->getPageIndex(static_cast<std::uint8_t>(room));
                if (isAllowed(t) && room >= 0 && idx >= 0)
                {
                    bg.LoadMapBinary(owner.GetMapBinaryPath(), idx * 0x100 + 0x10);
                    bg.DrawMapByName(owner.GetMapName(), SystemConfig::kTileSizeWidth, SystemConfig::kTileSizeHeight,
                        ox + pageW, oy);
                }
            }

            // 下
            if (oy + pageH < SystemConfig::kScreenHeight)
            {
                const int t = _scraper->getUnderScrollType(static_cast<int>(_pageIndex));
                const int16_t room = _scraper->getUnderRoom(static_cast<int>(_pageIndex));
                const int idx = _scraper->getPageIndex(static_cast<std::uint8_t>(room));
                if (isAllowed(t) && room >= 0 && idx >= 0)
                {
                    bg.LoadMapBinary(owner.GetMapBinaryPath(), idx * 0x100 + 0x10);
                    bg.DrawMapByName(owner.GetMapName(), SystemConfig::kTileSizeWidth, SystemConfig::kTileSizeHeight,
                        ox, oy + pageH);
                }
            }

            // 右下
            if (ox + pageW < SystemConfig::kScreenWidth && oy + pageH < SystemConfig::kScreenHeight)
            {
                const int tr = _scraper->getRightScrollType(static_cast<int>(_pageIndex));
                const int16_t rRoom = _scraper->getRightRoom(static_cast<int>(_pageIndex));
                const int rIdx = _scraper->getPageIndex(static_cast<std::uint8_t>(rRoom));
                if (isAllowed(tr) && rRoom >= 0 && rIdx >= 0)
                {
                    const int td = _scraper->getUnderScrollType(rIdx);
                    const int16_t rdRoom = _scraper->getUnderRoom(rIdx);
                    const int rdIdx = _scraper->getPageIndex(static_cast<std::uint8_t>(rdRoom));
                    if (isAllowed(td) && rdRoom >= 0 && rdIdx >= 0)
                    {
                        bg.LoadMapBinary(owner.GetMapBinaryPath(), rdIdx * 0x100 + 0x10);
                        bg.DrawMapByName(owner.GetMapName(), SystemConfig::kTileSizeWidth, SystemConfig::kTileSizeHeight,
                            ox + pageW, oy + pageH);
                    }
                }
            }

            // 左／上／左上も必要なら同様に（_camX/_camY が負寄りに振れるケースに備えて）
            if (ox > 0)
            {
                const int t = _scraper->getLeftScrollType(static_cast<int>(_pageIndex));
                const int16_t room = _scraper->getLeftRoom(static_cast<int>(_pageIndex));
                const int idx = _scraper->getPageIndex(static_cast<std::uint8_t>(room));
                if (isAllowed(t) && room >= 0 && idx >= 0)
                {
                    bg.LoadMapBinary(owner.GetMapBinaryPath(), idx * 0x100 + 0x10);
                    bg.DrawMapByName(owner.GetMapName(), SystemConfig::kTileSizeWidth, SystemConfig::kTileSizeHeight,
                        ox - pageW, oy);
                }
            }
            if (oy > 0)
            {
                const int t = _scraper->getOverScrollType(static_cast<int>(_pageIndex));
                const int16_t room = _scraper->getOverRoom(static_cast<int>(_pageIndex));
                const int idx = _scraper->getPageIndex(static_cast<std::uint8_t>(room));
                if (isAllowed(t) && room >= 0 && idx >= 0)
                {
                    bg.LoadMapBinary(owner.GetMapBinaryPath(), idx * 0x100 + 0x10);
                    bg.DrawMapByName(owner.GetMapName(), SystemConfig::kTileSizeWidth, SystemConfig::kTileSizeHeight,
                        ox, oy - pageH);
                }
            }
            if (ox > 0 && oy > 0)
            {
                const int tr = _scraper->getLeftScrollType(static_cast<int>(_pageIndex));
                const int16_t rRoom = _scraper->getLeftRoom(static_cast<int>(_pageIndex));
                const int rIdx = _scraper->getPageIndex(static_cast<std::uint8_t>(rRoom));
                if (isAllowed(tr) && rRoom >= 0 && rIdx >= 0)
                {
                    const int td = _scraper->getOverScrollType(rIdx);
                    const int16_t rdRoom = _scraper->getOverRoom(rIdx);
                    const int rdIdx = _scraper->getPageIndex(static_cast<std::uint8_t>(rdRoom));
                    if (isAllowed(td) && rdRoom >= 0 && rdIdx >= 0)
                    {
                        bg.LoadMapBinary(owner.GetMapBinaryPath(), rdIdx * 0x100 + 0x10);
                        bg.DrawMapByName(owner.GetMapName(), SystemConfig::kTileSizeWidth, SystemConfig::kTileSizeHeight,
                            ox - pageW, oy - pageH);
                    }
                }
            }
        }
    }
}