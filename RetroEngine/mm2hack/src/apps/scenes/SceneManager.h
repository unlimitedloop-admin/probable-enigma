//==============================================================================
// 
//  Project: mm2hack
//  SceneManager.h
// 
//  Scene management class that handles scene changes and updates.
// 
//==============================================================================
#pragma once

#include <memory>
#include "IBaseScene.h"

namespace mm2hack::apps::scenes
{
    // Scene management class (sample)
    class SceneManager final
    {
    public:
        void Update();
        void Release();

        // Transition to a new scene, releasing the current one if necessary (concept of state-pattern).
        void ChangeScene(std::unique_ptr<IBaseScene> newScene);

    private:
        std::unique_ptr<IBaseScene> _currentScene;
    };
}