//==============================================================================
// 
//  Project: mm2hack
//  FadeIOTexture.h
// 
//  Fade-in/out controlling class for texture variants (e.g., paletteVariants).
// 
//==============================================================================
#pragma once

#include <functional>
#include <optional>

namespace mm2hack::apps::rendering::effects
{
    // Texture variant fade-in/out controller
    class FadeIOTexture
    {
    public:
        enum class Curve { Linear, EaseIn, EaseOut, EaseInOut };

        struct Target
        {
            std::function<int()>        get;    // Get the current variant
            std::function<void(int)>    set;    // Set the variant
            std::function<int()>        max;    // Get the maximum variant (e.g., paletteVariants-1). Returns 0 if not set.
        };

        explicit FadeIOTexture(Target t, Curve c = Curve::Linear, int fallbackMax = 0);
        ~FadeIOTexture() = default;
        // Fade directly to v
        void BeginTo(int to, int frames);
        // Fade out: If to is not specified, use max() (or fallbackMax if not set)
        void FadeOut(int frames, std::optional<int> to = std::nullopt);
        // Fade in: To 0
        void FadeIn(int frames);
        // Cancel fade
        void Cancel();
        // Call every frame (if you want to do it over time, just hold dt)
        void Update();
        // Is it currently fading?
        bool IsActive() const noexcept;
        // Set the curve type
        void SetCurve(Curve c) noexcept;
        // Set the fallback maximum variant (used when max() is not set or returns 0)
        void SetFallbackMax(int v) noexcept;

    private:
        int MaxV_() const;                      // Get the maximum variant
        int ClampV_(int v) const;               // Clamp to [0, MaxV_()]
        static float Ease_(float t);            // Easing function

    private:
        const std::wstring kClassName = L"FadeIOTexture";

        Target _t;                              // Target texture variant
        Curve  _curve{ Curve::Linear };         // Easing curve
        int    _fallbackMax{ 0 };               // Fallback maximum variant

        bool   _active{ false };
        int    _from{ 0 }, _to{ 0 };
        int    _frames{ 0 }, _frame{ 0 };
    };
}