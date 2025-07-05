#include "SceneManager.h"

#include <memory>
#include <utility>
#include "IBaseScene.h"

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

    void SceneManager::ChangeScene(std::unique_ptr<IBaseScene> newScene)
    {
        if (_currentScene)
        {
            _currentScene->Finalize();
        }
        _currentScene = std::move(newScene);
    }
}