#include "pch.h"

#include "PhaseFadeController.h"

#include "apps/supervisor/ResourceManager.h"

namespace mm2hack::apps::scenes
{
    void PhaseFadeController::BeginPhase(const PhaseFadePlan& plan, ResourceManager& res)
    {
        _plan = plan;
        _state = State::PreBlackHold;
        _counter = 0;
        ForceDark_(res);    // Begin in black.
    }

    void PhaseFadeController::Update(ResourceManager& res)
    {
        switch (_state)
        {
        case State::PreBlackHold:
            if (_counter++ >= _plan.preBlackHold) { StartFadeIn_(res); }
            break;
        case State::FadingIn:
            if (_counter++ >= _plan.fadeInFrames) { _state = State::Interactive; _counter = 0; }
            break;
        case State::Interactive:
            // Available for input; waiting for RequestFadeOut() to be called.
            break;
        case State::PreFadeOutHold:
            if (_counter++ >= _plan.preFadeOutHold) { StartFadeOut_(res); }
            break;
        case State::FadingOut:
            if (_counter++ >= _plan.fadeOutFrames) { _state = State::PostBlackHold; _counter = 0; ForceDark_(res); }
            break;
        case State::PostBlackHold:
            if (_counter++ >= _plan.postBlackHold) { _state = State::Idle; }
            break;
        case State::Idle:
            break;
        }
    }

    void PhaseFadeController::RequestFadeOut(ResourceManager& res)
    {
        if (_state != State::Interactive) return;

        if (_plan.preFadeOutHold > 0)
        {
            _state = State::PreFadeOutHold;
            _counter = 0;
        }
        else
        {
            StartFadeOut_(res);
        }
    }

    void PhaseFadeController::StartFadeIn_(ResourceManager& res)
    {
        if (_plan.fadeInFrames > 0)
        {
            if (_plan.layers & FadeLayerMask::BG)     FadeInBG_(res, _plan.fadeInFrames);
            if (_plan.layers & FadeLayerMask::Sprite) FadeInSprite_(res, _plan.fadeInFrames);
            if (_plan.layers & FadeLayerMask::Font)   FadeInFont_(res, _plan.fadeInFrames);
        }
        else
        {
            LightUp_(res);
        }

        _state = State::FadingIn;
        _counter = 0;
    }

    void PhaseFadeController::StartFadeOut_(ResourceManager& res)
    {
        if (_plan.fadeOutFrames > 0)
        {
            if (_plan.layers & FadeLayerMask::BG)     FadeOutBG_(res, _plan.fadeOutFrames);
            if (_plan.layers & FadeLayerMask::Sprite) FadeOutSprite_(res, _plan.fadeOutFrames);
            if (_plan.layers & FadeLayerMask::Font)   FadeOutFont_(res, _plan.fadeOutFrames);
        }
        else
        {
            ForceDark_(res);
        }

        _state = State::FadingOut;
        _counter = 0;
    }

    void PhaseFadeController::FadeInBG_(ResourceManager& res, int to)
    {
        res.FadeInBG(to);
    }

    void PhaseFadeController::FadeInSprite_(ResourceManager& res, int to)
    {
        res.FadeInSprite(to);
    }

    void PhaseFadeController::FadeInFont_(ResourceManager& res, int to)
    {
        res.FadeInFont(to);
    }

    void PhaseFadeController::FadeOutBG_(ResourceManager& res, int to)
    {
        res.FadeOutBG(to);
    }

    void PhaseFadeController::FadeOutSprite_(ResourceManager& res, int to)
    {
        res.FadeOutSprite(to);
    }

    void PhaseFadeController::FadeOutFont_(ResourceManager& res, int to)
    {
        res.FadeOutFont(to);
    }

    void PhaseFadeController::ForceDark_(ResourceManager& res) const
    {
        if (_plan.layers & FadeLayerMask::BG)
        {
            auto& m = res.GetBGTileManager();
            m.SetGlobalVariantClamped(m.MaxVariant());
        }
        if (_plan.layers & FadeLayerMask::Sprite)
        {
            auto& m = res.GetSpriteManager();
            m.SetGlobalVariantClamped(m.MaxVariant());
        }
        if (_plan.layers & FadeLayerMask::Font)
        {
            auto& m = res.GetFontTileManager();
            m.SetGlobalVariantClamped(m.MaxVariant());
        }
    }

    void PhaseFadeController::LightUp_(ResourceManager& res) const
    {
        if (_plan.layers & FadeLayerMask::BG)
            res.GetBGTileManager().SetGlobalVariantClamped(0);

        if (_plan.layers & FadeLayerMask::Sprite)
            res.GetSpriteManager().SetGlobalVariantClamped(0);

        if (_plan.layers & FadeLayerMask::Font)
            res.GetFontTileManager().SetGlobalVariantClamped(0);
    }
}