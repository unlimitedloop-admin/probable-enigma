//==============================================================================
// 
//  Project: mm2hack
//  FadeIOTexture.h
// 
//  
// 
//==============================================================================
#pragma once

#include <algorithm>
#include <cmath>
#include <functional>
#include <optional>
#include <utility>

// TODO: Refactor implement method at cpp
// TODO: Translate to English comments
namespace mm2hack::apps::graphics::effects
{
    // Texture variant fade-in/out controller
    class FadeIOTexture
    {
    public:
        enum class Curve { Linear, EaseIn, EaseOut, EaseInOut };

        struct Target
        {
            std::function<int()>        get;   // Get the current variant
            std::function<void(int)>    set;   // Set the variant
            std::function<int()>        max;   // Get the maximum variant (e.g., paletteVariants-1). Returns 0 if not set.
        };

        explicit FadeIOTexture(Target t, Curve c = Curve::Linear, int fallbackMax = 0)
            : _t(std::move(t)), _curve(c), _fallbackMax(std::max(0, fallbackMax))
        {
        }

        // Fade directly to v
        void BeginTo(int to, int frames)
        {
            _active = true;
            _from = clampV(_t.get());
            _to = clampV(to);
            _frames = std::max(1, frames);
            _frame = 0;
        }

        // Fade out: If to is not specified, use max() (or fallbackMax if not set)
        void FadeOut(int frames, std::optional<int> to = std::nullopt)
        {
            const int dst = to ? *to : maxV();
            BeginTo(dst, frames);
        }

        // Fade in: To 0
        void FadeIn(int frames) { BeginTo(0, frames); }

        void Cancel() { _active = false; }

        // Call every frame (if you want to do it over time, just hold dt)
        void Update()
        {
            if (!_active) return;
            ++_frame;
            const float t = std::min(1.0f, float(_frame) / float(_frames));
            const float k = ease(t);
            const int v = clampV(int(std::lround((1.0f - k) * _from + k * _to)));
            _t.set(v);
            if (_frame >= _frames) _active = false;
        }

        bool IsActive() const noexcept { return _active; }
        void SetCurve(Curve c) noexcept { _curve = c; }
        void SetFallbackMax(int v) noexcept { _fallbackMax = std::max(0, v); }

    private:
        int maxV() const
        {
            if (_t.max)
            {
                int m = _t.max();
                if (m > 0) return m;
            }
            return _fallbackMax; // JSON not set / insurance during fluctuations
        }

        int clampV(int v) const
        {
            return std::max(0, std::min(v, maxV()));
        }

        static float ease(float t)
        {
            if (t <= 0.f) return 0.f; if (t >= 1.f) return 1.f;
            // otherwise, linear
            return t;
        }

    private:
        Target _t;
        Curve  _curve{ Curve::Linear };
        int    _fallbackMax{ 0 };

        bool   _active{ false };
        int    _from{ 0 }, _to{ 0 };
        int    _frames{ 0 }, _frame{ 0 };
    };
}