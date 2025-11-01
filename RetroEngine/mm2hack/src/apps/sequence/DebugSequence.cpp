#include "pch.h"

#include "DebugSequence.h"

#include "apps/foundation/NES/NESPalette.h"
#include "apps/resources/parameters/Parameters.h"
#include "apps/scenes/SceneID.h"
#include "apps/scenes/SceneManager.h"
#include "core/save/SaveData.h"
#include "SequenceType.h"

namespace mm2hack::apps::sequence
{
    DebugSequence::DebugSequence()
    {
        // Initialize the sequence, checks whether the resource can be loaded, etc.
        _sceneChanger.RegisterListener(&_sceneManager);
        _sceneManager.SetMediator(&_sceneChanger);

        // NOTE: LaunchingGame -> BackdoorMenu
        resources::parameters::Parameters params;
        params = params.With<scenes::SceneID>(L"Subsequent", scenes::SceneID::BackdoorMenu);
        _sceneChanger.RequestChange(scenes::SceneID::LaunchingGame, params);

        // Load the default background color for the NES palette.
        foundation::NES::NESPalette::SetBackgroundFor(config::SystemConfig::kMakeSeqPaletteIndex);
    }

    DebugSequence::~DebugSequence()
    {
        // Clean up resources, finalize the sequence, etc.
        _sceneManager.Release();
        foundation::NES::NESPalette::SetBackgroundFor(config::SystemConfig::kDefaultNESPaletteIndex);
    }

    void DebugSequence::Execute()
    {
        // Backdoor menu execution logic for debugging purposes only.
        _sceneManager.Update();
    }

    void DebugSequence::RenderWorld()
    {
        // Render the game world for the debug mode.
        _sceneManager.RenderWorld();
    }

    void DebugSequence::RenderOverlay()
    {
        // Render any overlays for the debug mode.
        _sceneManager.RenderOverlay();
    }

    scenes::SceneManager* DebugSequence::GetSceneManager()
    {
        return &_sceneManager;
    }

    bool DebugSequence::Save(core::save::SaveData& out) const
    {
        // Add more data to SaveData if needed. (Other managers, etc.)
        out.sequenceID = static_cast<int>(SequenceType::Debug);
        return true;
    }

    bool DebugSequence::Load(const core::save::SaveData& in)
    {
        return true;
    }
}