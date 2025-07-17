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
#include "apps/parameters/Parameters.h"
#include "IBaseScene.h"
#include "SceneID.h"

namespace mm2hack::apps::scenes
{
    // SceneManager is responsible for managing the current scene in the application
    class SceneManager final : public ISceneChangedListener
    {
    public:
        SceneManager() = default;
        ~SceneManager() override = default;

        // Updates the currentScene instance, standard game program execution procedures
        void Update();
        // Releases the currentScene
        void Release();
        // Retrieves the current scene ID as a integer
        int GetCurrentSceneID() const;
        // The role is to switch to a specified scene through an intermediary
        void RequestSceneChange(SceneID nextScene, const parameters::Parameters& params = {}) override;

    private:
        std::unique_ptr<IBaseScene> _currentScene;      // Manage the main part of the game program

        // Transition to a new scene, releasing the current one if necessary (concept of state-pattern)
        void ChangeScene(std::unique_ptr<IBaseScene> newScene);
    };
}