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
#include "apps/parameters/Parameters.h"
#include "apps/scenes/IBaseScene.h"
#include "apps/scenes/SceneChangeMediator.h"
#include "apps/scenes/SceneID.h"

namespace mm2hack::apps::scenes
{
    // This is scene that appears when the game is launching
    class LaunchingGame : public IBaseScene
    {
    public:
        LaunchingGame(SceneChangeMediator* mediator);
        LaunchingGame(const LaunchingGame&) = delete;
        LaunchingGame& operator=(const LaunchingGame&) = delete;
        LaunchingGame(LaunchingGame&&) = default;
        LaunchingGame& operator=(LaunchingGame&&) = default;
        ~LaunchingGame() override;

        // Initialize the scene with parameters
        void Initialize(const parameters::Parameters& params) override;
        // Finalize and clean up the scene
        void Finalize() override;
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
        const std::wstring kClassName{ L"LaunchingGame" };

        SceneChangeMediator* _mediator{ nullptr };      // Mediator for scene changes
        SceneID _subsequentScene{ SceneID::None };      // The scene to transition to after launching
    };
}