//==============================================================================
// 
//  Project: mm2hack
//  SceneManager.h
// 
//  Scene management class that handles scene changes and updates.
// 
//==============================================================================
#pragma once

#include "ISceneChangedListener.h"

#include <memory>
#include "IBaseScene.h"

namespace mm2hack::apps::resources::parameters
{
    class Parameters;
}

namespace mm2hack::apps::scenes
{
    class SceneChangeMediator;
    enum class SceneID : int;
}

namespace mm2hack::apps::scenes
{
    // SceneManager is responsible for managing the current scene in the application
    class SceneManager final : public ISceneChangedListener
    {
        using Parameters = resources::parameters::Parameters;

    public:
        SceneManager() = default;
        ~SceneManager() override = default;

        // Updates the currentScene instance, standard game program execution procedures
        void Update();
        inline void DoExecute() { Update(); }   // Alias for Update
        // Renders the currentScene instance
        void RenderWorld();
        // Renders the overlay of the application (e.g., HUD, menus)
        void RenderOverlay();

        // Releases the currentScene
        void Release();
        // Retrieves the current scene ID as a integer
        int GetCurrentSceneID() const;
        // The role is to switch to a specified scene through an intermediary
        void RequestSceneChange(SceneID nextScene, const Parameters& params = {}) override;
        // Sets the mediator for scene changes
        void SetMediator(SceneChangeMediator* mediator) { _mediator = mediator; }

    private:
        SceneChangeMediator* _mediator = nullptr;                   // Intermediary for scene changes
        std::unique_ptr<IBaseScene> _currentScene;                  // Manage the main part of the game program

        void changeScene_(std::unique_ptr<IBaseScene> newScene);    // Transition to a new scene, releasing the current one if necessary(concept of state-pattern)
    };
}