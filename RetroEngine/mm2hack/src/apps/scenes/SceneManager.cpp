#include "pch.h"

#include "SceneManager.h"

#include <sstream>
#include <string>
#include "apps/runtime/GameContext.h"
#include "core/overlay/PauseManager.h"
#include "core/save/SaveData.h"
#include "IBaseScene.h"
#include "SceneFactory.h"
#include "sub-scenes/90-BackdoorMenu/BackdoorMenu.h"

namespace mm2hack::apps::scenes
{
    void SceneManager::Update()
    {
        using namespace core::overlay;
        if (_currentScene)
        {
            auto& time = runtime::GameContext::GetInstance().Time();
            if (!PauseManager::IsPaused() || (time.DeltaSeconds() > 0.0))   // No-op if paused and not stepping one frame
            {
                _currentScene->Update();        // !Execute the main game logic.
            }

            if (PauseManager::IsPaused())
            {
                PauseManager::DrawOverlay();    // Draw a "PAUSED" overlay.
            }
        }
    }

    void SceneManager::RenderWorld()
    {
        if (_currentScene)
        {
            _currentScene->RenderWorld();
        }
    }

    void SceneManager::RenderOverlay()
    {
        if (_currentScene)
        {
            // It's drawn after RenderWorld, so it appears on top of everything else.
            _currentScene->RenderOverlay();
        }
    }

    void SceneManager::Release()
    {
        _currentScene.reset();
    }

    int SceneManager::GetCurrentSceneID() const
    {
        if (_currentScene)
        {
            return static_cast<int>(_currentScene->GetSceneID());
        }
        return -1;
    }

    bool SceneManager::SaveState(core::save::SaveData& out) const
    {
        if (!_currentScene || !_currentScene->CanSaveState())
        {
            return false;
        }

        std::ostringstream payload(std::ios::out | std::ios::binary);
        if (!_currentScene->Save(payload) || !payload.good())
        {
            return false;
        }

        const std::string bytes = payload.str();
        out.sceneID = static_cast<std::int32_t>(_currentScene->GetSceneID());
        out.scenePayload.assign(bytes.begin(), bytes.end());
        return true;
    }

    bool SceneManager::LoadState(const core::save::SaveData& in)
    {
        const auto scene_id = static_cast<SceneID>(in.sceneID);
        if (scene_id == SceneID::None)
        {
            return false;
        }

        if (!_currentScene || _currentScene->GetSceneID() != scene_id)
        {
            RequestSceneChange(scene_id, {});
        }
        if (!_currentScene || _currentScene->GetSceneID() != scene_id)
        {
            return false;
        }

        const std::string bytes(in.scenePayload.begin(), in.scenePayload.end());
        std::istringstream payload(bytes, std::ios::in | std::ios::binary);
        if (!_currentScene->Load(payload))
        {
            return false;
        }
        return payload.peek() == std::char_traits<char>::eof();
    }

    bool SceneManager::ValidateState(const core::save::SaveData& in)
    {
        const auto scene_id = static_cast<SceneID>(in.sceneID);
        const std::string bytes(in.scenePayload.begin(), in.scenePayload.end());
        std::istringstream payload(bytes, std::ios::in | std::ios::binary);

        bool valid = false;
        switch (scene_id)
        {
        case SceneID::BackdoorMenu:
            valid = BackdoorMenu::ValidateState(payload);
            break;
        case SceneID::DemoStage2:
            // The complete action-stage DTO is introduced incrementally by
            // DS2-003 through DS2-006. Keep the load gate closed until then.
            return false;
        default:
            return false;
        }
        return valid && payload.peek() == std::char_traits<char>::eof();
    }

    void SceneManager::RequestSceneChange(SceneID nextScene, const Parameters& params)
    {
        // Create the next scene instance using the SceneFactory.
        auto next = SceneFactory::CreateScene(nextScene, _mediator);
        if (!next)
        {
            return;
        }

        // Finalize the current scene before switching to the next one.
        if (_currentScene)
        {
            _currentScene->Finalize();
        }

        next->Initialize(params);
        _currentScene = std::move(next);
    }
}
