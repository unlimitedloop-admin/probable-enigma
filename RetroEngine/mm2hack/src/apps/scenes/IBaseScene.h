//==============================================================================
// 
//  Project: mm2hack
//  IBaseScene.h
// 
//  A generic interface for scene managing the main game program on-the-fly.
// 
//==============================================================================
#pragma once

#include <string>
#include "SceneID.h"

namespace mm2hack::apps::scenes
{
    // Interface for scene management in the sequence
    class IBaseScene
    {
    public:
        virtual ~IBaseScene() = default;

        virtual void Initialize() = 0;
        virtual void Finalize() = 0;
        virtual void Update() = 0;

        virtual SceneID GetSceneID() const = 0;
        virtual std::wstring GetSceneName() const = 0;
    };
}