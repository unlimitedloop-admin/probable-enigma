//==============================================================================
// 
//  Project: mm2hack
//  SceneFactory.h
// 
//  Factory of scenes that creates instances of IBaseScene based on SceneID.
// 
//==============================================================================
#pragma once

#include <memory>
#include "IBaseScene.h"
#include "SceneChangeMediator.h"
#include "SceneID.h"

namespace mm2hack::apps::scenes
{
    // Factory class for creating scenes based on SceneID
    class SceneFactory
    {
    public:
        // Creates a scene instance based on the given SceneID and mediator
        static std::unique_ptr<IBaseScene> CreateScene(SceneID id, SceneChangeMediator* mediator);
    };
}