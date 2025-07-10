#include "DebugSequence.h"

#include "apps/scenes/SceneManager.h"
#include "core/save/SaveData.h"
#include "SequenceType.h"

namespace mm2hack::apps::sequence
{
    DebugSequence::DebugSequence()
    {
        // Initialize the sequence, load resources, etc.
    }

    DebugSequence::~DebugSequence()
    {
        // Clean up resources, finalize the sequence, etc.
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