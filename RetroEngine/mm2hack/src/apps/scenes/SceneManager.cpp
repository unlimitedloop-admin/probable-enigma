#include "pch.h"

#include "SceneManager.h"

#include "apps/runtime/GameContext.h"
#include "core/overlay/PauseManager.h"
#include "IBaseScene.h"
#include "SceneFactory.h"

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