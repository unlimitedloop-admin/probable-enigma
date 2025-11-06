#include "pch.h"

#include "SceneFactory.h"

#include "IBaseScene.h"
#include "SceneChangeMediator.h"
#include "SceneID.h"

// Add more sub-scenes here...
#include "sub-scenes/01-DemoStage1/DemoStage1.h"
#include "sub-scenes/90-BackdoorMenu/BackdoorMenu.h"
#include "sub-scenes/LaunchingGame.h"

namespace mm2hack::apps::scenes
{
    std::unique_ptr<IBaseScene> SceneFactory::CreateScene(SceneID id, SceneChangeMediator* mediator)
    {
        switch (id)
        {
        case SceneID::LaunchingGame:
            return std::make_unique<LaunchingGame>(mediator);
        case SceneID::BackdoorMenu:
            return std::make_unique<BackdoorMenu>(mediator);
        case SceneID::DemoStage1:
            return std::make_unique<DemoStage1>(mediator);

            // Add more cases for other scenes as needed...

        default:
            return nullptr;
        }
    }
}