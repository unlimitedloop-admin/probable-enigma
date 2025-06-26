#include "StandardSequence.h"

#include "apps/scenes/SceneManager.h"

namespace mm2hack::apps::sequence
{
    StandardSequence::StandardSequence()
    {
        // Initialize the sequence, load resources, etc.
        // HACK: mediator->RegisterListener(&_sceneManager);
        // RequestChange(scenes::SceneID::<SceneClass>);
        // state pattern...
    }

    StandardSequence::~StandardSequence()
    {
        _sceneManager.Release();
    }

    void StandardSequence::Execute()
    {
        // Main execution logic for the standard game mode.
        _sceneManager.Update();
    }

    scenes::SceneManager* StandardSequence::GetSceneManager()
    {
        return &_sceneManager;
    }
}