//==============================================================================
// 
//  Project: mm2hack
//  LaunchingGame.h
// 
//  Header file for the LaunchingGame scene.
// 
//==============================================================================
#pragma once

#include <string>
#include "apps/scenes/IBaseScene.h"

namespace mm2hack::apps::resources::parameters
{
    class Parameters;
}

namespace mm2hack::apps::scenes
{
    class SceneChangeMediator;
}

namespace mm2hack::apps::scenes
{
    // This is scene that appears when the game is launching
    class LaunchingGame : public IBaseScene
    {
        using Parameters = resources::parameters::Parameters;

    public:
        LaunchingGame(SceneChangeMediator* mediator);
        LaunchingGame(const LaunchingGame&) = delete;
        LaunchingGame& operator=(const LaunchingGame&) = delete;
        LaunchingGame(LaunchingGame&&) = default;
        LaunchingGame& operator=(LaunchingGame&&) = default;
        ~LaunchingGame() override;

        // Update the scene logic for each frame
        void Update() override;
        // Render the main game world, draw the sprites and BGs
        void RenderWorld() override {}
        // Render the overlay, draw the HUD and menus
        void RenderOverlay() override {}

        // Get the unique identifier for this scene
        SceneID GetSceneID() const override { return SceneID::LaunchingGame; }
        // Get the name of this scene as a wstring
        std::wstring GetSceneName() const override { return kClassName; }

    private:
        // Initialize the scene with parameters
        void onEnter_(const Parameters& params) override;
        // Finalize and clean up the scene
        void onExit_() override;

    private:
        const std::wstring kClassName{ L"LaunchingGame" };

        SceneChangeMediator* _mediator{ nullptr };      // Mediator for scene changes
        SceneID _subsequentScene{ SceneID::None };      // The scene to transition to after launching
    };
}