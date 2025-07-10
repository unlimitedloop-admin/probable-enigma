//==============================================================================
// 
//  Project: mm2hack
//  ISceneChangedListener.h
// 
//  Scene change listener interface.
// 
//==============================================================================
#pragma once

#include "apps/parameters/Parameters.h"
#include "SceneID.h"

namespace mm2hack::apps::scenes
{
    // Interface used for switching scenes
    class ISceneChangedListener
    {
    public:
        virtual ~ISceneChangedListener() = default;
        virtual void RequestSceneChange(SceneID nextScene, const parameters::Parameters& params) = 0;
    };
}