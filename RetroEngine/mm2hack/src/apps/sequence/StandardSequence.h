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

#include "apps/scenes/SceneChangeMediator.h"
#include "apps/scenes/SceneManager.h"
#include "core/save/SaveData.h"

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
        // Override the RenderWorld method to render the game world
        void RenderWorld() override;
        // Override the RenderOverlay method to render any overlays
        void RenderOverlay() override;
        // Get the scene manager instance
        scenes::SceneManager* GetSceneManager() override;
        // Assign the main data of the sequence to the SaveData structure
        bool Save(core::save::SaveData& out) const override;
        // Load the main data of the sequence from the SaveData structure
        bool Load(const core::save::SaveData& in) override;

    private:
        scenes::SceneManager _sceneManager;         // Scene manager instance
        scenes::SceneChangeMediator _sceneChanger;  // Scene change mediator instance
    };
}