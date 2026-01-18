//==============================================================================
// 
//  Project: mm2hack
//  IBaseScene.h
// 
//  A generic interface for scene managing the main game program on-the-fly.
// 
//==============================================================================
#pragma once

#include <cassert>
#include <string>
#include "apps/resources/parameters/Parameters.h"

namespace mm2hack::apps::scenes
{
    // Enum class for all scene IDs
    enum class SceneID
    {
        None = -1,
        LaunchingGame = 0,
        DemoStage1,
        DemoStage2,
        Opening,
        BackdoorMenu = 90,      // Backdoor menu for debugging purposes only (A list of selectable scenes)
        // Add more scenes as needed
    };

    // Interface for scene management in the sequence
    class IBaseScene
    {
    public:
        virtual ~IBaseScene() noexcept = default;

        //------------------------------------------------------------------------------
        // Lifecycle API (non-virtual)
        //------------------------------------------------------------------------------
        void Initialize(const resources::parameters::Parameters& params)
        {
            // Initialize() is intended to be called once per instance.
            // In release build, ignore repeated calls to avoid crashing the app.
            // In debug build, assert to catch misuse early.
            assert(!_is_entered && "Initialize() called more than once.");
            if (_is_entered)
            {
                return;
            }

            // If already finalized, entering makes no sense.
            assert(!_is_exited && "Initialize() called after Finalize().");
            if (_is_exited)
            {
                return;
            }

            _is_entered = true;
            onEnter_(params);
        }

        void Finalize() noexcept
        {
            // Finalize() is always safe:
            // - callable even if Initialize() was never called
            // - callable multiple times (idempotent)

            if (_is_exited)
            {
                return;
            }

            _is_exited = true;

            // Only call OnExit_() when the scene actually entered.
            // This avoids "exit without enter" bugs and keeps OnExit_() simpler.
            if (_is_entered)
            {
                onExit_();
            }
        }

        //------------------------------------------------------------------------------
        // Main scene interface
        //------------------------------------------------------------------------------
        virtual void Update() = 0;
        virtual void RenderWorld() = 0;
        virtual void RenderOverlay() = 0;

        // === Scene identification ===
        virtual SceneID GetSceneID() const = 0;
        virtual std::wstring GetSceneName() const = 0;

        [[nodiscard]] bool IsEntered() const noexcept { return _is_entered; }
        [[nodiscard]] bool IsExited() const noexcept { return _is_exited; }

    protected:
        IBaseScene() = default;

        //------------------------------------------------------------------------------
        // Hooks implemented by derived scenes
        //------------------------------------------------------------------------------
        virtual void onEnter_(const resources::parameters::Parameters& params) = 0;
        virtual void onExit_() = 0;

    private:
        bool _is_entered = false;
        bool _is_exited = false;
    };
}