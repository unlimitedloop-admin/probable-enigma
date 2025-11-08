#include "pch.h"

#include "BgWobble2D.h"

#include <cmath>
#include <numbers>

// TODO: 日本語は使用禁止。全部英語で。
namespace mm2hack::apps::systems::scrolling::effect
{
    void BgWobble2D::Initialize(int logical_h, int stripe_h) noexcept
    {
        _logical_h = logical_h;
        _stripe_h = (stripe_h <= 0) ? 1 : stripe_h;

        const int stripes = (_logical_h + _stripe_h - 1) / _stripe_h;
        _offx.assign(static_cast<std::size_t>(stripes), 0.0f);
        _srcy.assign(static_cast<std::size_t>(stripes), 0);
    }

    void BgWobble2D::SetParams(float ax, float ay, float freq, float spd, float phase0, float edge) noexcept
    {
        _amp_x_px = ax;
        _amp_y_px = ay;
        _freq_y = freq;   // unused (kept for compatibility)
        _speed = spd;
        _phase = phase0;
        _edge_atten = std::clamp(edge, 0.0f, 1.0f); // unused (kept for compatibility)
    }

    void BgWobble2D::SetRasterBanding(int band_h_px, float phase_step_rad, bool bottom_to_top) noexcept
    {
        _band_h_px = (band_h_px <= 0) ? 0 : band_h_px; // 0 is invalid
        _band_phase_step = phase_step_rad;
        _band_bottom_to_top = bottom_to_top;
    }

    void BgWobble2D::Update(float dt) noexcept
    {
        _phase += _speed * dt;
        constexpr float two_pi = std::numbers::pi_v<float> *2.0f;
        if (_phase > two_pi || _phase < -two_pi)
        {
            _phase = std::fmod(_phase, two_pi);
        }
    }

    // Additional: Feel free to make it a class member if you prefer. Here, it's a local constant for minimal diff.
    // Central emphasis (1.0 = conventional / 1.2 to 1.6 for deeper center)
    constexpr float kGammaX = 1.00f; // Horizontal U central emphasis
    constexpr float kGammaY = 1.43f; // Vertical "Y error" central emphasis

    void BgWobble2D::Render(int src, int src_w, int src_h,
        int dst_w, int dst_h,
        float dst_x, float dst_y) noexcept
    {
        if (src < 0 || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) { return; }

        const int stripes = static_cast<int>(_offx.size());
        const float base_t = _phase;
        const float phi = std::numbers::pi_v<float> *1.0f; // 8-shaped bias (as before)

        // Split the screen into vertical bands (for phase delay): treat 0 as 1 band
        const int band_h = (_band_h_px > 0) ? _band_h_px : _logical_h;
        const int bands = (band_h > 0) ? ((_logical_h + band_h - 1) / band_h) : 1;

        // Smudge reduction (integer rounding) — only round dx to avoid breaking existing artwork
        auto roundi = [](float v) noexcept { return std::floor(v + 0.5f); };

        for (int i = 0; i < stripes; ++i)
        {
            const int   y0 = i * _stripe_h;
            const float v = static_cast<float>(y0) / static_cast<float>(_logical_h); // 0..1（上→下）

            // Central maximum weight (same shape as before) + gamma emphasis
            //   wx: horizontal U width strength, wy: vertical Y error strength (both maximum at center, minimum at edges)
            float baseW = uParabola_(v);                 // 0..1, center=1
            float wx = std::pow(baseW, kGammaX);
            float wy = std::pow(baseW, kGammaY);

            // ----- Band index and delay phase (as before) -----
            const int band_idx_from_top = (band_h > 0) ? (y0 / band_h) : 0;      // 0=topmost
            const int band_idx_from_bottom = (bands - 1) - band_idx_from_top;
            const int band_idx = _band_bottom_to_top ? band_idx_from_bottom : band_idx_from_top;

            const float t_band = base_t + _band_phase_step * static_cast<float>(band_idx);

            // ----- U-shaped horizontal (wide) + 8-shaped vertical (deeper center) -----
            const float offx = _amp_x_px * std::sinf(t_band) * wx;  // Horizontal with lag
            const float offy = _amp_y_px * std::sinf(2.0f * t_band + phi) * wy;  // Vertical with lag

            _offx[static_cast<std::size_t>(i)] = offx;

            int sy = y0 + static_cast<int>(std::lround(offy));
            sy = std::clamp(sy, 0, src_h - std::min(_stripe_h, src_h));
            _srcy[static_cast<std::size_t>(i)] = sy;
        }

        // Scale (as before, assuming 1:1 means sx, sy = 1)
        const float sx = static_cast<float>(dst_w) / static_cast<float>(src_w);
        const float sy = static_cast<float>(dst_h) / static_cast<float>(src_h);

        // Stripe transfer (as before, only round dx)
        for (int i = 0; i < stripes; ++i)
        {
            const int y0 = i * _stripe_h;
            const int h = (y0 + _stripe_h <= src_h) ? _stripe_h : (src_h - y0);
            if (h <= 0) { continue; }

            const float ox = _offx[static_cast<std::size_t>(i)];
            const int   sySrc = _srcy[static_cast<std::size_t>(i)];

            const float dx1 = roundi(dst_x + ox * sx);
            const float dy1 = dst_y + static_cast<float>(y0) * sy;
            const float dx2 = dx1 + static_cast<float>(src_w) * sx;
            const float dy2 = dy1 + static_cast<float>(h) * sy;

            ::DxLib::DrawRectGraphF(dx1, dy1, 0, sySrc, src_w, h, src, TRUE);
        }
    }

    float BgWobble2D::uParabola_(float v01) const noexcept
    {
        // U(v) = 1 - 4*(v-0.5)^2, clamp to [0,1]
        const float d = v01 - 0.5f;
        const float w = 1.0f - 4.0f * d * d;
        return (w > 0.0f) ? w : 0.0f;
    }
}