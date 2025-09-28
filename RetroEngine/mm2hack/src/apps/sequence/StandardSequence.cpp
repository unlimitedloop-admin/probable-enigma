#include "pch.h"

#include "StandardSequence.h"

#include "apps/NES/NESPalette.h"
#include "apps/parameters/Parameters.h"
#include "apps/scenes/SceneChangeMediator.h"
#include "apps/scenes/SceneID.h"
#include "apps/scenes/SceneManager.h"
#include "core/save/SaveData.h"
#include "SequenceType.h"

namespace mm2hack::apps::sequence
{
    StandardSequence::StandardSequence()
    {
        // Initialize the sequence, checks whether the resource can be loaded, etc.
        _sceneChanger.RegisterListener(&_sceneManager);
        _sceneManager.SetMediator(&_sceneChanger);

        // NOTE: LaunchingGame -> Opening
        parameters::Parameters params;
        params = params.With<scenes::SceneID>(L"Subsequent", scenes::SceneID::Opening);
        _sceneChanger.RequestChange(scenes::SceneID::LaunchingGame, params);

        // Load the default background color for the NES palette.
        NES::NESPalette::SetBackgroundFor(config::SystemConfig::kMakeSeqPaletteIndex);
    }

    StandardSequence::~StandardSequence()
    {
        // Clean up resources, finalize the sequence, etc.
        _sceneManager.Release();
        NES::NESPalette::SetBackgroundFor(config::SystemConfig::kDefaultNESPaletteIndex);
    }

    void StandardSequence::Execute()
    {
        // Main execution logic for the standard game mode.
        _sceneManager.Update();
    }

    void StandardSequence::RenderWorld()
    {
        // Render the game world for the standard game mode.
        _sceneManager.RenderWorld();
    }

    void StandardSequence::RenderOverlay()
    {
        // Render any overlays for the standard game mode.
        _sceneManager.RenderOverlay();
    }

    scenes::SceneManager* StandardSequence::GetSceneManager()
    {
        return &_sceneManager;
    }

    bool StandardSequence::Save(core::save::SaveData& out) const
    {
        // Add more data to SaveData if needed. (Other managers, etc.)
        out.sequenceID = static_cast<int>(SequenceType::Standard);
        return true;
    }

    bool StandardSequence::Load(const core::save::SaveData& in)
    {
        return true;
    }
}