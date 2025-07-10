#include "StandardSequence.h"

#include "apps/scenes/SceneChangeMediator.h"
#include "apps/scenes/SceneID.h"
#include "apps/scenes/SceneManager.h"
#include "core/save/SaveData.h"
#include "SequenceType.h"

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