#include "pch.h"

#include "ScrollController.h"

#include <cstdint>
#include "apps/mod/CoordinateTypes.h"
#include "ScrollTypes.h"

namespace mm2hack::apps::algorithm::scrolling::atomic
{
    static inline int MaxX(int view_w) { return view_w - 1; }
    static inline int MaxY(int view_h) { return view_h - 1; }

    void ScrollController::Update(const mod::Vec2& input_delta)
    {
        const int page_w = _params.tile_px * _tileX;
        const int page_h = _params.tile_px * _tileY;

        // 固定ページアニメ中：進捗のみ
        if (_anim.Active())
        {
            const auto dir = _anim.State().dir;
            const bool finished = _anim.TickAndInterpolate(dir, page_w, page_h, _params.view_w, _params.view_h, _object_pos);
            if (finished)
            {
                _page_index = _anim.State().to_index; // 対岸へ確定
                _cam.x = 0.0; _cam.y = 0.0;
                _anim.Reset();
            }
            return;
        }

        // 自由スクロール
        if (input_delta.x != 0.0) { UpdateAxisX(input_delta.x); }
        if (input_delta.y != 0.0) { UpdateAxisY(input_delta.y); }
    }

    void ScrollController::Render()
    {
        const int page_w = _params.tile_px * _tileX;
        const int page_h = _params.tile_px * _tileY;

        if (_anim.Active())
        {
            // 固定ページスクロール演出中
            _renderer.DrawAnimation(_anim.State(), _anim.State().from_index, _anim.State().to_index);
        }
        else
        {
            // 自由スクロール中：現ページ + 必要分のみ隣接ページ
            const int ox = -static_cast<int>(_cam.x);
            const int oy = -static_cast<int>(_cam.y);
            _renderer.DrawPage(_page_index, ox, oy);
            DrawNeighbors();
        }

        // デバッグ十字線
        ::DxLib::DrawLine(static_cast<int>(_object_pos.x), 0, static_cast<int>(_object_pos.x), _params.view_h, 0xFFFF0000, 2);
        ::DxLib::DrawLine(0, static_cast<int>(_object_pos.y), _params.view_w, static_cast<int>(_object_pos.y), 0xFFFF0000, 2);
    }

    // ───────────── X 軸処理
    void ScrollController::UpdateAxisX(double remain)
    {
        const int page_w = _params.tile_px * 16;
        // 1) 十字線→中央へ寄せる
        if (remain > 0.0 && _object_pos.x < Camera::kCenterX)
        {
            const double take = std::min(remain, Camera::kCenterX - _object_pos.x);
            _object_pos.x += take; remain -= take;
        }
        else if (remain < 0.0 && _object_pos.x > Camera::kCenterX)
        {
            const double take = std::min(-remain, _object_pos.x - Camera::kCenterX);
            _object_pos.x -= take; remain += take;
        }

        if (Camera::NearlyZero(remain)) { return; }

        if (remain > 0.0) // →
        {
            const double tentative = _cam.x + remain;
            if (tentative <= 0.0)
            {
                _cam.x = tentative; _object_pos.x = Camera::kCenterX; return;
            }

            const auto kind = _rules.RightType(_page_index);
            const auto rr = _rules.RightRoom(_page_index);
            const int r_idx = (rr >= 0) ? _rules.ToPageIndex(static_cast<uint8_t>(rr)) : -1;

            if (IsAllowedFree(kind) && r_idx >= 0)
            {
                _cam.x = tentative; _object_pos.x = Camera::kCenterX;
                while (_cam.x >= page_w) { _page_index = static_cast<std::size_t>(r_idx); _cam.x -= page_w; }
            }
            else if (kind == ScrollKind::FixedPage && r_idx >= 0)
            {
                const int max_x = MaxX(_params.view_w);
                double need = static_cast<double>(max_x) - _object_pos.x;
                if (need > 0.0)
                {
                    const double step = std::min(remain, need);
                    _object_pos.x += step; remain -= step;
                }
                if (_object_pos.x >= max_x && remain > 0.0)
                {
                    _anim.Start(PageScroll::Dir::Right, _page_index, static_cast<std::size_t>(r_idx));
                    _cam.x = 0.0;
                }
            }
            else
            {
                // 不許可：線だけ動かす
                _object_pos.x = std::clamp(_object_pos.x + remain, 0.0, static_cast<double>(MaxX(_params.view_w)));
            }
        }
        else // ←
        {
            const double tentative = _cam.x + remain;
            if (tentative >= 0.0)
            {
                _cam.x = tentative; _object_pos.x = Camera::kCenterX; return;
            }

            const auto kind = _rules.LeftType(_page_index);
            const auto lr = _rules.LeftRoom(_page_index);
            const int l_idx = (lr >= 0) ? _rules.ToPageIndex(static_cast<uint8_t>(lr)) : -1;

            if (IsAllowedFree(kind) && l_idx >= 0)
            {
                _cam.x = tentative; _object_pos.x = Camera::kCenterX;
                while (_cam.x < 0.0) { _page_index = static_cast<std::size_t>(l_idx); _cam.x += page_w; }
            }
            else if (kind == ScrollKind::FixedPage && l_idx >= 0)
            {
                const int min_x = 0;
                double need = _object_pos.x - static_cast<double>(min_x);
                if (need > 0.0)
                {
                    const double step = std::min(-remain, need);
                    _object_pos.x -= step; remain += step;
                }
                if (_object_pos.x <= min_x && remain < 0.0)
                {
                    _anim.Start(PageScroll::Dir::Left, _page_index, static_cast<std::size_t>(l_idx));
                    _cam.x = 0.0;
                }
            }
            else
            {
                _object_pos.x = std::clamp(_object_pos.x + remain, 0.0, static_cast<double>(MaxX(_params.view_w)));
            }
        }
    }

    // ───────────── Y 軸処理
    void ScrollController::UpdateAxisY(double remain)
    {
        const int page_h = _params.tile_px * 15;

        if (remain > 0.0 && _object_pos.y < Camera::kCenterY)
        {
            const double take = std::min(remain, Camera::kCenterY - _object_pos.y);
            _object_pos.y += take; remain -= take;
        }
        else if (remain < 0.0 && _object_pos.y > Camera::kCenterY)
        {
            const double take = std::min(-remain, _object_pos.y - Camera::kCenterY);
            _object_pos.y -= take; remain += take;
        }

        if (Camera::NearlyZero(remain)) { return; }

        if (remain > 0.0) // ↓
        {
            const double tentative = _cam.y + remain;
            if (tentative <= 0.0)
            {
                _cam.y = tentative; _object_pos.y = Camera::kCenterY; return;
            }

            const auto kind = _rules.DownType(_page_index);
            const auto dr = _rules.DownRoom(_page_index);
            const int d_idx = (dr >= 0) ? _rules.ToPageIndex(static_cast<uint8_t>(dr)) : -1;

            if (IsAllowedFree(kind) && d_idx >= 0)
            {
                _cam.y = tentative; _object_pos.y = Camera::kCenterY;
                while (_cam.y >= page_h) { _page_index = static_cast<std::size_t>(d_idx); _cam.y -= page_h; }
            }
            else if (kind == ScrollKind::FixedPage && d_idx >= 0)
            {
                const int max_y = MaxY(_params.view_h);
                double need = static_cast<double>(max_y) - _object_pos.y;
                if (need > 0.0)
                {
                    const double step = std::min(remain, need);
                    _object_pos.y += step; remain -= step;
                }
                if (_object_pos.y >= max_y && remain > 0.0)
                {
                    _anim.Start(PageScroll::Dir::Down, _page_index, static_cast<std::size_t>(d_idx));
                    _cam.y = 0.0;
                }
            }
            else
            {
                _object_pos.y = std::clamp(_object_pos.y + remain, 0.0, static_cast<double>(MaxY(_params.view_h)));
            }
        }
        else // ↑
        {
            const double tentative = _cam.y + remain;
            if (tentative >= 0.0)
            {
                _cam.y = tentative; _object_pos.y = Camera::kCenterY; return;
            }

            const auto kind = _rules.UpType(_page_index);
            const auto ur = _rules.UpRoom(_page_index);
            const int u_idx = (ur >= 0) ? _rules.ToPageIndex(static_cast<uint8_t>(ur)) : -1;

            if (IsAllowedFree(kind) && u_idx >= 0)
            {
                _cam.y = tentative; _object_pos.y = Camera::kCenterY;
                while (_cam.y < 0.0) { _page_index = static_cast<std::size_t>(u_idx); _cam.y += page_h; }
            }
            else if (kind == ScrollKind::FixedPage && u_idx >= 0)
            {
                const int min_y = 0;
                double need = _object_pos.y - static_cast<double>(min_y);
                if (need > 0.0)
                {
                    const double step = std::min(-remain, need);
                    _object_pos.y -= step; remain += step;
                }
                if (_object_pos.y <= min_y && remain < 0.0)
                {
                    _anim.Start(PageScroll::Dir::Up, _page_index, static_cast<std::size_t>(u_idx));
                    _cam.y = 0.0;
                }
            }
            else
            {
                _object_pos.y = std::clamp(_object_pos.y + remain, 0.0, static_cast<double>(MaxY(_params.view_h)));
            }
        }
    }

    void ScrollController::DrawNeighbors()
    {
        const int page_w = _params.tile_px * 16;
        const int page_h = _params.tile_px * 15;

        const int ox = -static_cast<int>(_cam.x);
        const int oy = -static_cast<int>(_cam.y);

        const auto is_allowed = [](ScrollKind k) { return IsAllowedFree(k); };

        // 右
        if (ox + page_w < _params.view_w)
        {
            const auto k = _rules.RightType(_page_index);
            const int16_t room = _rules.RightRoom(_page_index);
            const int idx = _rules.ToPageIndex(static_cast<uint8_t>(room));
            if (is_allowed(k) && room >= 0 && idx >= 0)
            {
                _renderer.DrawPage(static_cast<std::size_t>(idx), ox + page_w, oy);
            }
        }
        // 下
        if (oy + page_h < _params.view_h)
        {
            const auto k = _rules.DownType(_page_index);
            const int16_t room = _rules.DownRoom(_page_index);
            const int idx = _rules.ToPageIndex(static_cast<uint8_t>(room));
            if (is_allowed(k) && room >= 0 && idx >= 0)
            {
                _renderer.DrawPage(static_cast<std::size_t>(idx), ox, oy + page_h);
            }
        }
        // 右下
        if (ox + page_w < _params.view_w && oy + page_h < _params.view_h)
        {
            const auto kr = _rules.RightType(_page_index);
            const int16_t rr = _rules.RightRoom(_page_index);
            const int r_idx = _rules.ToPageIndex(static_cast<uint8_t>(rr));
            if (IsAllowedFree(kr) && rr >= 0 && r_idx >= 0)
            {
                const auto kd = _rules.DownType(static_cast<std::size_t>(r_idx));
                const int16_t rd = _rules.DownRoom(static_cast<std::size_t>(r_idx));
                const int rd_idx = _rules.ToPageIndex(static_cast<uint8_t>(rd));
                if (IsAllowedFree(kd) && rd >= 0 && rd_idx >= 0)
                {
                    _renderer.DrawPage(static_cast<std::size_t>(rd_idx), ox + page_w, oy + page_h);
                }
            }
        }
        // 左
        if (ox > 0)
        {
            const auto k = _rules.LeftType(_page_index);
            const int16_t room = _rules.LeftRoom(_page_index);
            const int idx = _rules.ToPageIndex(static_cast<uint8_t>(room));
            if (is_allowed(k) && room >= 0 && idx >= 0)
            {
                _renderer.DrawPage(static_cast<std::size_t>(idx), ox - page_w, oy);
            }
        }
        // 上
        if (oy > 0)
        {
            const auto k = _rules.UpType(_page_index);
            const int16_t room = _rules.UpRoom(_page_index);
            const int idx = _rules.ToPageIndex(static_cast<uint8_t>(room));
            if (is_allowed(k) && room >= 0 && idx >= 0)
            {
                _renderer.DrawPage(static_cast<std::size_t>(idx), ox, oy - page_h);
            }
        }
        // 左上
        if (ox > 0 && oy > 0)
        {
            const auto kl = _rules.LeftType(_page_index);
            const int16_t lr = _rules.LeftRoom(_page_index);
            const int l_idx = _rules.ToPageIndex(static_cast<uint8_t>(lr));
            if (IsAllowedFree(kl) && lr >= 0 && l_idx >= 0)
            {
                const auto ku = _rules.UpType(static_cast<std::size_t>(l_idx));
                const int16_t lu = _rules.UpRoom(static_cast<std::size_t>(l_idx));
                const int lu_idx = _rules.ToPageIndex(static_cast<uint8_t>(lu));
                if (IsAllowedFree(ku) && lu >= 0 && lu_idx >= 0)
                {
                    _renderer.DrawPage(static_cast<std::size_t>(lu_idx), ox - page_w, oy - page_h);
                }
            }
        }
    }
}