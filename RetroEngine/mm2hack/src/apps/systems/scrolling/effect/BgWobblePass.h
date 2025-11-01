//==============================================================================
// 
//  Project: mm2hack
//  BgWobblePass.h
// 
//  Scene-local BG wobble composite (CPU, no shader).
// 
//==============================================================================
#pragma once

#include "BgWobble2D.h"

namespace mm2hack::apps::systems::scrolling::effect
{
    // Scene-local BG wobble composite (CPU, no shader)
    class BgWobblePass
    {
    public:
        BgWobblePass() = default;
        ~BgWobblePass();

        // logical_w/h : your NES-like resolution (e.g., 256x240)
        // stripe_h    : 1..4 (1 = per scanline, 2~4 = faster)
        bool Initialize(int logical_w, int logical_h, int stripe_h = 2) noexcept;
        // Recca-style "U" initial values (change according to preference)
        void SetParams(float amp_x_px, float amp_y_px, float freq_y, float speed, float phase0, float edge_atten) noexcept;

        // Begin rendering to the BG-only render target
        void Begin() const noexcept;
        // Update internal state with delta time in seconds
        void Update(float deltaTimeSec) noexcept;
        // End rendering and composite to specified render target with wobble effect
        void EndAndCompositeToRT(int dst_handle, int dst_w, int dst_h, float dt) noexcept;
        // Raster vertical band phase delay settings
        void SetRasterBanding(int band_h_px, float phase_step_rad, bool bottom_to_top) noexcept;

        [[nodiscard]] inline int GetBgRenderTarget() const noexcept { return _bg_rt; }

    private:
        int _logical_w{ 0 };    // e.g. 256
        int _logical_h{ 0 };    // e.g. 240
        int _bg_rt{ -1 };       // BG-only render target

        BgWobble2D _wobble;     // BG wobble effect instance
    };
}