//==============================================================================
// 
//  Project: mm2hack
//  StandardSequence.h
// 
//  This sequence implements the standard game mode.
// 
//==============================================================================
#pragma once

#include "ISequence.h"

#include "apps/scenes/SceneManager.h"

namespace mm2hack::apps::sequence
{
    // StandardSequence class implements the standard game mode
    class StandardSequence : public ISequence
    {
    public:
        StandardSequence();
        ~StandardSequence() override;
        // Override the Execute method to implement the sequence logic
        void Execute() override;
        scenes::SceneManager* GetSceneManager() override;

    private:
        scenes::SceneManager _sceneManager;     // Scene manager instance
    };
}