#include "pch.h"

#include "FixedScrollDriver.h"

#include <algorithm>
#include "ScrollController.h" // if Camera/ScrollEffect are in there; otherwise include their headers.
#include "ScrollFreezeState.h"
#include "ScrollNeighborResolver.h"
#include "ScrollTypes.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    bool FixedScrollDriver::Request(const FixedScrollRequest& req) noexcept
    {
        if (req.dir == PageScroll::Dir::None)
        {
            return false;
        }

        if (_animator.Active() || _pending.has_value())
        {
            return false;
        }

        _pending = req;
        return true;
    }

    bool FixedScrollDriver::Update(int page_w, int page_h, Camera& cam, std::size_t& page_index, ScrollFreezeState& freeze, ScrollEffect& out_fx) noexcept
    {
        // Freeze has the highest priority (fixed scroll lock)
        if (freeze.IsActive())
        {
            freeze.Tick();
            return true;
        }

        // Active animation
        if (_animator.Active())
        {
            tickAnim_(page_w, page_h, cam, page_index, freeze, out_fx);
            return true;
        }

        // Start pending request (before free-scroll)
        if (_pending.has_value())
        {
            const FixedScrollRequest req = *_pending;
            _pending.reset();

            if (tryStart_(req, page_index, page_w, page_h, cam))
            {
                _carry_total_px = req.carryTotalPx;
                freeze.BeginStartFreeze();
                return true;
            }
        }

        return false;   // NOTE: Fixed scroll inactive! (free scroll may proceed by ScrollController)
    }

    bool FixedScrollDriver::tryStart_(const FixedScrollRequest& req, std::size_t& page_index, int page_w, int page_h, Camera& cam) noexcept
    {
        (void)page_w;
        (void)page_h;

        if (req.dir == PageScroll::Dir::None)
        {
            return false;
        }

        const auto from_index = page_index;
        const std::optional<std::size_t> to = _resolver.ResolveFixedNeighbor(req.dir, from_index);
        if (!to.has_value())
        {
            return false;
        }

        cam.x = 0.0;
        cam.y = 0.0;

        _animator.Start(req.dir, from_index, *to);
        return true;
    }

    void FixedScrollDriver::tickAnim_(int page_w, int page_h, Camera& cam, std::size_t& page_index, ScrollFreezeState& freeze, ScrollEffect& out_fx) noexcept
    {
        using ScrlDir = PageScroll::Dir;

        out_fx.fixedActive = true;

        const ScrlDir dir = _animator.State().dir;
        const double prev_prog = _animator.State().progress;

        const bool finished = _animator.Tick(dir, page_w, page_h);

        const double need = (dir == ScrlDir::Left || dir == ScrlDir::Right)
            ? static_cast<double>(page_w)
            : static_cast<double>(page_h);

        const double curr_prog = finished ? need : std::min(_animator.State().progress, need);
        const double d_prog = std::max(0.0, curr_prog - prev_prog);

        const double carry = (need > 0.0) ? (d_prog * (_carry_total_px / need)) : 0.0;

        switch (dir)
        {
        case ScrlDir::Right: out_fx.playerDelta.x = +carry; break;
        case ScrlDir::Left:  out_fx.playerDelta.x = -carry; break;
        case ScrlDir::Down:  out_fx.playerDelta.y = +carry; break;
        case ScrlDir::Up:    out_fx.playerDelta.y = -carry; break;
        default: break;
        }

        if (finished)
        {
            finish_(need, cam, page_index, freeze);
        }
    }

    void FixedScrollDriver::finish_(double need, Camera& cam, std::size_t& page_index, ScrollFreezeState& freeze) noexcept
    {
        PageScroll snap = _animator.State();
        snap.progress = need;
        snap.active = true;

        freeze.SetDrawSnapshot(snap);
        freeze.BeginEndFreeze();

        page_index = _animator.State().to_index;

        cam.x = 0.0;
        cam.y = 0.0;

        _animator.Reset();
        _carry_total_px = 0.0;
    }
}