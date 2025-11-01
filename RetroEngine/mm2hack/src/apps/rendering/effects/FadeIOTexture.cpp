#include "pch.h"

#include "FadeIOTexture.h"

#include <cmath>
#include <optional>

namespace mm2hack::apps::rendering::effects
{
    FadeIOTexture::FadeIOTexture(Target t, Curve c, int fallbackMax)
        : _t(std::move(t)), _curve(c), _fallbackMax(std::max(0, fallbackMax))
    {
    }

    void FadeIOTexture::BeginTo(int to, int frames)
    {
        _active = true;
        _from = ClampV_(_t.get());
        _to = ClampV_(to);
        _frames = std::max(1, frames);
        _frame = 0;
    }

    void FadeIOTexture::FadeOut(int frames, std::optional<int> to)
    {
        const int dst = to ? *to : MaxV_();
        BeginTo(dst, frames);
    }

    void FadeIOTexture::FadeIn(int frames) { BeginTo(0, frames); }

    void FadeIOTexture::Cancel() { _active = false; }

    void FadeIOTexture::Update()
    {
        if (!_active) return;
        ++_frame;
        const float t = std::min(1.0f, float(_frame) / float(_frames));
        const float k = Ease_(t);
        const int v = ClampV_(int(std::lround((1.0f - k) * _from + k * _to)));
        _t.set(v);
        if (_frame >= _frames) _active = false;
    }

    bool FadeIOTexture::IsActive() const noexcept { return _active; }

    void FadeIOTexture::SetCurve(Curve c) noexcept { _curve = c; }

    void FadeIOTexture::SetFallbackMax(int v) noexcept { _fallbackMax = std::max(0, v); }

    int FadeIOTexture::MaxV_() const
    {
        if (_t.max)
        {
            int m = _t.max();
            if (m > 0) return m;
        }
        return _fallbackMax; // JSON not set / insurance during fluctuations
    }

    int FadeIOTexture::ClampV_(int v) const
    {
        return std::max(0, std::min(v, MaxV_()));
    }

    float FadeIOTexture::Ease_(float t)
    {
        if (t <= 0.f) return 0.f; if (t >= 1.f) return 1.f;
        // otherwise, linear
        return t;
    }
}