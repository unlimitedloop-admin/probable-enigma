//==============================================================================
// 
//  Project: mm2hack
//  ResourceFadeBindings.h
// 
//  Performs fade-in and fade-out effects on resources such as assets.
// 
//==============================================================================
#pragma once

#include <optional>
#include "apps/graphics/bg/BGTileManager.h"
#include "apps/graphics/effects/FadeIOTexture.h"
#include "apps/graphics/fonts/FontTileManager.h"
#include "apps/graphics/sprite/SpriteManager.h"

namespace mm2hack::apps::supervisor
{
    // Bindings for fading resources (sprite, bg tile, font tile)
    class ResourceFadeBindings
    {
    public:
        using Fade = apps::graphics::effects::FadeIOTexture;

        ResourceFadeBindings(graphics::sprite::SpriteManager& sm,
            graphics::bg::BGTileManager& bm,
            graphics::fonts::FontTileManager& fm,
            int fallbackSpriteMax = 3,
            int fallbackBGMax = 3,
            int fallbackFontMax = 3)
            : _fadeSprite({
                // get
                [&sm] { return sm.GlobalVariant(); },
                // set
                [&sm](int v) { sm.SetGlobalVariant(v); },
                // max
                [&sm] { return sm.MaxVariant(); }
            }, Fade::Curve::Linear, fallbackSpriteMax)
            , _fadeBG({
                // get
                [&bm] { return bm.GlobalVariant(); },
                // set
                [&bm](int v) { bm.SetGlobalVariant(v); },
                // max
                [&bm] { return bm.MaxVariant(); }
            }, Fade::Curve::Linear, fallbackBGMax)
            , _fadeFont({
                // get
                [&fm] { return fm.GlobalVariant(); },
                // set
                [&fm](int v) { fm.SetGlobalVariant(v); },
                // max
                [&fm] { return fm.MaxVariant(); }
            }, Fade::Curve::Linear, fallbackFontMax)
        {
        }

        void Update() { _fadeSprite.Update(); _fadeBG.Update(); _fadeFont.Update(); }

        // Shortcuts
        void FadeOutBG(int frames, std::optional<int> to = std::nullopt) { _fadeBG.FadeOut(frames, to); }
        void FadeInBG(int frames) { _fadeBG.FadeIn(frames); }
        void FadeOutSprite(int frames, std::optional<int> to = std::nullopt) { _fadeSprite.FadeOut(frames, to); }
        void FadeInSprite(int frames) { _fadeSprite.FadeIn(frames); }
        void FadeOutFont(int frames, std::optional<int> to = std::nullopt) { _fadeFont.FadeOut(frames, to); }
        void FadeInFont(int frames) { _fadeFont.FadeIn(frames); }

        // Animation curves
        void SetBGCurve(Fade::Curve c) { _fadeBG.SetCurve(c); }
        void SetSpriteCurve(Fade::Curve c) { _fadeSprite.SetCurve(c); }
        void SetFontCurve(Fade::Curve c) { _fadeFont.SetCurve(c); }

        // Overwrite if the upper limit is known after asset loading
        void SetFallbackSpriteMax(int v) { _fadeSprite.SetFallbackMax(v); }
        void SetFallbackBGMax(int v) { _fadeBG.SetFallbackMax(v); }
        void SetFallbackFontMax(int v) { _fadeFont.SetFallbackMax(v); }

    private:
        Fade _fadeSprite{ {[]() { return 0; }, [](int) {}, []() { return 0; } }, Fade::Curve::Linear, 0 };  // Sprite fade controller
        Fade _fadeBG{ {[]() { return 0; }, [](int) {}, []() { return 0; } }, Fade::Curve::Linear, 0 };      // BG tile fade controller
        Fade _fadeFont{ {[]() { return 0; }, [](int) {}, []() { return 0; } }, Fade::Curve::Linear, 0 };    // Font tile fade controller
    };
}