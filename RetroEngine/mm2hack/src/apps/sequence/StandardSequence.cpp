#include "StandardSequence.h"

#include <memory>
#include "apps/scenes/SceneChangeMediator.h"
#include "apps/scenes/SceneID.h"
#include "apps/scenes/SceneManager.h"

namespace mm2hack::apps::sequence
{
    StandardSequence::StandardSequence()
    {
        _sceneChanger.RegisterListener(&_sceneManager);
        // NOTE: This defines the first scene to be executed.
        _sceneChanger.RequestChange(scenes::SceneID::LaunchingGame);
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