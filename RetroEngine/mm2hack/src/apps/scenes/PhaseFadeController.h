//==============================================================================
// 
//  Project: mm2hack
//  PhaseFadeController.h
// 
//  A fade control class that allows detailed setting of fade-in/out transitions.
// 
//==============================================================================
#pragma once

#include <cstdint>
#include "apps/supervisor/ResourceManager.h"

namespace mm2hack::apps::scenes
{
    // Layers that can be faded in/out
    struct FadeLayerMask
    {
        enum : uint8_t { BG = 1, Sprite = 2, Font = 4, All = BG | Sprite | Font };
    };

    // Plan for phase fade transitions
    struct PhaseFadePlan
    {
        int preBlackHold = 0;   // Hold time at the start of the phase, fully black
        int fadeInFrames = 30;  // Fade-in duration
        int preFadeOutHold = 0; // Hold time before fade-out (not black)
        int fadeOutFrames = 30; // Fade-out duration
        int postBlackHold = 0;  // Hold time after fade-out, fully black
        uint8_t layers = FadeLayerMask::BG | FadeLayerMask::Sprite | FadeLayerMask::Font; // Target layers
    };

    // Controller for phase fade transitions
    class PhaseFadeController
    {
    public:
        using ResourceManager = supervisor::ResourceManager;
        enum class State { Idle, PreBlackHold, FadingIn, Interactive, PreFadeOutHold, FadingOut, PostBlackHold };

        PhaseFadeController() = default;
        ~PhaseFadeController() = default;

        // Start a new phase with the given plan
        void BeginPhase(const PhaseFadePlan& plan, ResourceManager& res);
        // Update the fade state each frame
        void Update(ResourceManager& res);
        // Request fade-out (only effective when in Interactive state)
        void RequestFadeOut(ResourceManager& res);

        [[nodiscard]] inline bool InputEnabled() const noexcept { return _state == State::Interactive; }
        [[nodiscard]] inline bool ReadyToSwitchPhase() const noexcept { return _state == State::Idle; }
        [[nodiscard]] inline State Current() const noexcept { return _state; }
        [[nodiscard]] inline const PhaseFadePlan& Plan() const noexcept { return _plan; }

    private:
        void StartFadeIn_(ResourceManager& res);
        void StartFadeOut_(ResourceManager& res);

        void FadeInBG_(ResourceManager& res, int to);
        void FadeInSprite_(ResourceManager& res, int to);
        void FadeInFont_(ResourceManager& res, int to);
        void FadeOutBG_(ResourceManager& res, int to);
        void FadeOutSprite_(ResourceManager& res, int to);
        void FadeOutFont_(ResourceManager& res, int to);

        void ForceDark_(ResourceManager& res) const;
        void LightUp_(ResourceManager& res) const;

    private:
        PhaseFadePlan _plan{};          // Current fade plan
        State _state{ State::Idle };    // Current state
        int _counter{ 0 };              // Frame counter for the current state
    };

}