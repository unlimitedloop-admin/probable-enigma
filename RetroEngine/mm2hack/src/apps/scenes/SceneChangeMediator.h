//==============================================================================
// 
//  Project: mm2hack
//  SceneChangeMediator.h
// 
//  It's an intermediary that manages scene transitions independent of the SceneManager.
// 
//==============================================================================
#pragma once

#include <string>
#include "apps/resources/parameters/Parameters.h"

namespace mm2hack::apps::scenes
{
    class ISceneChangedListener;
    enum class SceneID : int;
}

namespace mm2hack::apps::scenes
{
    // Mediator for scene changes, allowing a single listener to handle requests
    // NOTE: Dependency Inversion is used here, so that the SceneChangeMediator does not depend on the SceneManager.
    class SceneChangeMediator final
    {
        using Parameters = resources::parameters::Parameters;

    public:
        // Registers a listener to handle scene change requests
        void RegisterListener(ISceneChangedListener* listener);
        // Requests a scene change through the registered listener
        void RequestChange(SceneID scene, Parameters params = {});

    private:
        const std::wstring kClassName = L"SceneChangeMediator";

        ISceneChangedListener* _listener = nullptr;     // Pointer to the registered listener
    };
}