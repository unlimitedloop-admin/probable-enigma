#include "pch.h"

#include "TwinkleCursorAnimator.h"

#include <iterator>
#include <string_view>
#include "apps/rendering/sprite/SpriteManager.h"

namespace mm2hack::apps::vfx::cursor
{
    TwinkleCursorAnimator::TwinkleCursorAnimator(SpriteManager& sprites, int x, int y) noexcept
        : _sprites(sprites), _x(x), _y(y)
    {
    }

    bool TwinkleCursorAnimator::Load(const std::wstring& name, std::wstring_view png_path, std::wstring_view json_path)
    {
        _name = name;
        _id = _sprites.Load(_name, png_path, json_path);
        const bool ok = (_id >= 0);
        if (ok) { Reset(); }
        return ok;
    }

    void TwinkleCursorAnimator::SetPosition(int x, int y) noexcept
    {
        _x = x; _y = y;
    }

    void TwinkleCursorAnimator::Reset() noexcept
    {
        _stepIndex = 0U;
        _ticks = 0;
    }

    void TwinkleCursorAnimator::Update() noexcept
    {
        if (_id < 0 || _steps.empty()) { return; }

        // Process the current step.
        ++_ticks;
        if (_ticks >= _steps[_stepIndex].duration)
        {
            _ticks = 0;
            _stepIndex = (_stepIndex + 1U) % _steps.size();
        }
    }

    void TwinkleCursorAnimator::Draw() const noexcept { drawImpl_(_x, _y); }

    void TwinkleCursorAnimator::DrawAt(int x, int y) const noexcept { drawImpl_(x, y); }

    void TwinkleCursorAnimator::drawImpl_(int x, int y) const noexcept
    {
        if (_id < 0 || _steps.empty()) { return; }
        _sprites.UseById(_id, _steps[_stepIndex].tile, x, y);
    }

    void TwinkleCursorAnimator::SetLoop(const Step* steps, std::size_t count) noexcept
    {
        if (steps == nullptr || count == 0U)
        {
            _steps.assign(std::begin(kDefaultFadeLoop), std::end(kDefaultFadeLoop));
        }
        else
        {
            _steps.assign(steps, steps + count);
        }
        Reset();
    }

    void TwinkleCursorAnimator::SetBaseTileDuration(int frames) noexcept
    {
        if (frames <= 0 || _steps.empty()) { return; }
        for (auto& s : _steps)
        {
            if (s.tile == 0) { s.duration = frames; }
        }
    }
}