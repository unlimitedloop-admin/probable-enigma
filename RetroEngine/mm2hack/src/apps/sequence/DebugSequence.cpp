#include "DebugSequence.h"

#include "apps/scenes/SceneManager.h"

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
}