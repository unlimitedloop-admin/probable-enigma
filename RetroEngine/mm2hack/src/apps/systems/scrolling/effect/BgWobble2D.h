//==============================================================================
// 
//  Project: mm2hack
//  BgWobble2D.h
// 
//  NES-style BG "U-shaped wobble" + raster phase delay (CPU synthesis).
//  t_band = phase + band_phase_step * band_index (choice of down->up or up->down)
// 
//==============================================================================
#pragma once

#include <vector>

namespace mm2hack::apps::systems::scrolling::effect
{
    // A special effects that applies a pseudo-LissajousCurve scrolling effect to the background (BG) layer
    class BgWobble2D
    {
    public:
        BgWobble2D() = default;
        ~BgWobble2D() = default;

        // logical_h : logical screen height (e.g. 240)
        // stripe_h  : 1..4 (1 = per scanline; 2~4 for performance)
        void Initialize(int logical_h, int stripe_h) noexcept;

        // amplitudes in px; freq is kept for compatibility (unused); speed=rad/sec; phase0=rad
        // edge_atten is also kept for compatibility (unused)
        void SetParams(float amp_x_px, float amp_y_px, float freq, float speed, float phase0, float edge_atten) noexcept;

        // Raster vertical band phase delay settings
        // band_h_px        : e.g. 16 (0/negative is invalid = no band delay)
        // phase_step_rad   : phase added for each band up (rad)
        // bottom_to_top    : true=delay from bottom to top, false=top to bottom
        void SetRasterBanding(int band_h_px, float phase_step_rad, bool bottom_to_top) noexcept;

        // Update internal state with delta time in seconds
        void Update(float dt_sec) noexcept;

        // Copy BG from src_handle (logical size src_w*src_h) to BACKBUFFER with wobble and scaling
        void Render(int src_handle, int src_w, int src_h, int dst_w, int dst_h, float dst_x = 0.0f, float dst_y = 0.0f) noexcept;

    private:
        int   _stripe_h{ 2 };       // 1..4 (1=per scanline; 2~4 for performance)
        int   _logical_h{ 0 };      // e.g. 240

        float _amp_x_px{ 14.0f };
        float _amp_y_px{ 2.0f };
        float _freq_y{ 0.0f };      // unused (kept for compatibility)
        float _speed{ 2.6f };
        float _phase{ 0.0f };
        float _edge_atten{ 0.10f }; // unused (kept for compatibility)

        // Raster vertical band delay
        int   _band_h_px{ 16 };     // 16px per band
        float _band_phase_step{ 0.0f };
        bool  _band_bottom_to_top{ true };

        std::vector<float> _offx;   // per stripe X offset
        std::vector<int>   _srcy;   // per stripe source Y

        // U-shaped profile (center 0, max at top/bottom)
        [[nodiscard]] float UParabola(float v01) const noexcept;
    };
}