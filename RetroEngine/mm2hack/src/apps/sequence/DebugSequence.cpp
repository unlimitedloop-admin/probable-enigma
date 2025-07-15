#include "DebugSequence.h"

#include "apps/NES/NESPalette.h"
#include "apps/scenes/SceneManager.h"
#include "config/SystemConfig.h"
#include "core/save/SaveData.h"
#include "SequenceType.h"

namespace mm2hack::apps::sequence
{
    DebugSequence::DebugSequence()
    {
        // Initialize the sequence, load resources, etc.
        NES::NESPalette::SetBackgroundFor(config::SystemConfig::kMakeSeqPaletteIndex);
    }

    DebugSequence::~DebugSequence()
    {
        // Clean up resources, finalize the sequence, etc.
        NES::NESPalette::SetBackgroundFor(config::SystemConfig::kDefaultNESPaletteIndex);
    }

    void DebugSequence::Execute()
    {
        // Backdoor menu execution logic for debugging purposes only.
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