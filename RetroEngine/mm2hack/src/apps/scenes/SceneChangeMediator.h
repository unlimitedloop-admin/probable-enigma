//==============================================================================
// 
//  Project: mm2hack
//  SceneChangeMediator.h
// 
//  It's an intermediary that manages scene transitions independent of the SceneManager.
// 
//==============================================================================
#pragma once

#include "apps/parameters/Parameters.h"
#include "ISceneChangedListener.h"
#include "SceneID.h"

namespace mm2hack::apps::scenes
{
    // Mediator for scene changes, allowing a single listener to handle requests
    // NOTE: Dependency Inversion is used here, so that the SceneChangeMediator does not depend on the SceneManager.
    class SceneChangeMediator final
    {
    public:
        void RegisterListener(ISceneChangedListener* listener);
        void RequestChange(SceneID scene, const parameters::Parameters& params = {});

    private:
        ISceneChangedListener* _listener = nullptr;
    };
}