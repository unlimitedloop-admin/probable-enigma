#include "pch.h"

#include "SceneManager.h"

#include <utility>
#include "apps/deal/GameContext.h"
#include "apps/parameters/Parameters.h"
#include "core/overlay/PauseManager.h"
#include "IBaseScene.h"
#include "SceneFactory.h"
#include "SceneID.h"

namespace mm2hack::apps::scenes
{
    void SceneManager::Update()
    {
        using namespace core::overlay;
        if (_currentScene)
        {
            auto& time = deal::GameContext::GetInstance().Time();
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
        using namespace core::overlay;
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

    void SceneManager::RequestSceneChange(SceneID nextScene, const parameters::Parameters& params)
    {
        auto next = SceneFactory::CreateScene(nextScene, _mediator);
        if (next)
        {
            next->Initialize(params);
            ChangeScene(std::move(next));
        }
    }

    void SceneManager::ChangeScene(std::unique_ptr<IBaseScene> newScene)
    {
        if (_currentScene)
        {
            _currentScene->Finalize();
        }
        _currentScene = std::move(newScene);
    }
}