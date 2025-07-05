#include "StandardSequence.h"

#include <memory>
#include "apps/scenes/SceneManager.h"
#include "apps/scenes/sub-scenes/LaunchingGame.h"

namespace mm2hack::apps::sequence
{
    StandardSequence::StandardSequence()
    {
        // NOTE: This defines the first scene to be executed.
        _sceneManager.ChangeScene(std::make_unique<scenes::LaunchingGame>());
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