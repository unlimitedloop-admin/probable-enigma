#include "SceneManager.h"

#include <memory>
#include <utility>
#include "apps/parameters/Parameters.h"
#include "IBaseScene.h"
#include "SceneFactory.h"
#include "SceneID.h"

namespace mm2hack::apps::scenes
{
    void SceneManager::Update()
    {
        if (_currentScene)
        {
            _currentScene->Update();
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
        auto next = SceneFactory::CreateScene(nextScene);

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