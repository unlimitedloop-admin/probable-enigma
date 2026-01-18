#include "pch.h"

#include "StandardSequence.h"

#include "apps/foundation/NES/NESPalette.h"
#include "apps/resources/parameters/Parameters.h"
#include "apps/scenes/IBaseScene.h"
#include "apps/scenes/SceneManager.h"
#include "core/save/SaveData.h"
#include "SequenceManager.h"

namespace mm2hack::apps::sequence
{
    StandardSequence::StandardSequence()
    {
        using SceneID = scenes::SceneID;

        // Initialize the sequence, checks whether the resource can be loaded, etc.
        _sceneChanger.RegisterListener(&_sceneManager);
        _sceneManager.SetMediator(&_sceneChanger);

        // NOTE: LaunchingGame -> Opening
        resources::parameters::Parameters params;
        params = params.With<SceneID>(L"Subsequent", SceneID::Opening);
        _sceneChanger.RequestChange(SceneID::LaunchingGame, params);

        // Load the default background color for the NES palette.
        foundation::NES::NESPalette::SetBackgroundFor(config::SystemConfig::kMakeSeqPaletteIndex);
    }

    StandardSequence::~StandardSequence()
    {
        // Clean up resources, finalize the sequence, etc.
        _sceneManager.Release();
        foundation::NES::NESPalette::SetBackgroundFor(config::SystemConfig::kDefaultNESPaletteIndex);
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