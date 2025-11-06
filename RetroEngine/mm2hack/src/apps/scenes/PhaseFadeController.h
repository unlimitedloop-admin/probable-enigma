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
#include <string>
#include "apps/resources/ResourceManager.h"

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
        using ResourceManager = apps::resources::ResourceManager;

    public:
        enum class State { Idle, PreBlackHold, FadingIn, Interactive, PreFadeOutHold, FadingOut, PostBlackHold };

        PhaseFadeController() = default;
        ~PhaseFadeController() = default;

        // Start a new phase with the given plan
        void BeginPhase(const PhaseFadePlan& plan, ResourceManager& res);
        // Update the fade state each frame
        void Update(ResourceManager& res);
        // Request fade-out (only effective when in Interactive state)
        void RequestFadeOut(ResourceManager& res);

        // ----- Getters -----
        [[nodiscard]] inline bool InputEnabled() const noexcept { return _state == State::Interactive; }
        [[nodiscard]] inline bool ReadyToSwitchPhase() const noexcept { return _state == State::Idle; }
        [[nodiscard]] inline State Current() const noexcept { return _state; }
        [[nodiscard]] inline const PhaseFadePlan& Plan() const noexcept { return _plan; }

    private:
        void startFadeIn_(ResourceManager& res);            // Start fade-in process
        void startFadeOut_(ResourceManager& res);           // Start fade-out process

        // ----- Fade-in/out helpers -----
        void fadeInBG_(ResourceManager& res, int to);
        void fadeInSprite_(ResourceManager& res, int to);
        void fadeInFont_(ResourceManager& res, int to);
        void fadeOutBG_(ResourceManager& res, int to);
        void fadeOutSprite_(ResourceManager& res, int to);
        void fadeOutFont_(ResourceManager& res, int to);

        void forceDark_(ResourceManager& res) const;        // Force all target layers to black
        void lightUp_(ResourceManager& res) const;          // Restore all target layers to normal brightness

    private:
        const std::wstring kClassName = L"PhaseFadeController";

        PhaseFadePlan _plan{};          // Current fade plan
        State _state{ State::Idle };    // Current state
        int _counter{ 0 };              // Frame counter for the current state
    };
}