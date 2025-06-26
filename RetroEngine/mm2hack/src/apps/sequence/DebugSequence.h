//==============================================================================
// 
//  Project: mm2hack
//  DebugSequence.h
// 
//  This sequence implements the developer mode of the game.
// 
//==============================================================================
#pragma once

#include "ISequence.h"

#include "apps/scenes/SceneManager.h"

namespace mm2hack::apps::sequence
{
    // DebugSequence class implements the developer mode
    class DebugSequence : public ISequence
    {
    public:
        DebugSequence();
        ~DebugSequence() override;
        // Override the Execute method to implement the sequence logic
        void Execute() override;
        scenes::SceneManager* GetSceneManager() override;

    private:
        scenes::SceneManager _sceneManager;     // Scene manager instance
    };
}