//==============================================================================
// 
//  Project: mm2hack
//  ISceneChangedListener.h
// 
//  Scene change listener interface.
// 
//==============================================================================
#pragma once

#include "apps/resources/parameters/Parameters.h"
#include "IBaseScene.h"
//#include "SceneID.h"

namespace mm2hack::apps::scenes
{
    // Interface used for switching scenes
    class ISceneChangedListener
    {
    public:
        virtual ~ISceneChangedListener() = default;

        // Request a scene change
        virtual void RequestSceneChange(SceneID nextScene, const resources::parameters::Parameters& params) = 0;
    };
}