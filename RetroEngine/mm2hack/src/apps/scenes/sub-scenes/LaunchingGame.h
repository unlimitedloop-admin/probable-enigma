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
#include "apps/scenes/IBaseScene.h"
#include "apps/scenes/SceneID.h"

namespace mm2hack::apps::scenes
{
    class LaunchingGame : public IBaseScene
    {
    public:
        LaunchingGame();
        ~LaunchingGame() override;

        void Initialize() override;
        void Finalize() override;
        void Update() override;

        SceneID GetSceneID() const override;
        std::wstring GetSceneName() const override;
    };
}