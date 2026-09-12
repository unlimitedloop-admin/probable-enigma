#include "pch.h"

#include "BgWobblePass.h"

#include <DxLib.h>
#include <Windows.h>

namespace mm2hack::apps::systems::scrolling::effect
{
    BgWobblePass::~BgWobblePass()
    {
        if (_bg_rt >= 0) { ::DxLib::DeleteGraph(_bg_rt); }
    }

    bool BgWobblePass::Initialize(int logical_w, int logical_h, int stripe_h) noexcept
    {
        _logical_w = logical_w;
        _logical_h = logical_h;

        if (_bg_rt >= 0)
        {
            ::DxLib::DeleteGraph(_bg_rt);
            _bg_rt = -1;
        }

        _bg_rt = ::DxLib::MakeScreen(_logical_w, _logical_h, TRUE);
        if (_bg_rt < 0) { return false; }

        _wobble.Initialize(_logical_h, stripe_h);
        return true;
    }

    void BgWobblePass::SetParams(float amp_x_px, float amp_y_px, float freq_y, float speed, float phase0, float edge_atten) noexcept
    {
        _wobble.SetParams(amp_x_px, amp_y_px, freq_y, speed, phase0, edge_atten);
    }

    void BgWobblePass::Begin() const noexcept
    {
        ::DxLib::SetDrawScreen(_bg_rt);
        ::DxLib::ClearDrawScreen();
    }

    void BgWobblePass::Update(float deltaTimeSec) noexcept
    {
        _wobble.Update(deltaTimeSec);
    }

    void BgWobblePass::EndAndCompositeToRT(int dst_handle, int dst_w, int dst_h, float dt) noexcept
    {
        ::DxLib::SetDrawScreen(dst_handle);
        ::DxLib::SetDrawMode(DX_DRAWMODE_NEAREST);
        _wobble.Render(
            /*src*/ _bg_rt,
            /*src_w, src_h*/ _logical_w, _logical_h,
            /*dst_w, dst_h*/ dst_w, dst_h,
            /*dst_x, dst_y*/ 0.0f, 0.0f
        );
    }

    void BgWobblePass::SetRasterBanding(int band_h_px, float phase_step_rad, bool bottom_to_top) noexcept
    {
        _wobble.SetRasterBanding(band_h_px, phase_step_rad, bottom_to_top);
    }
}