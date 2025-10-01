#include "pch.h"

#include "SceneFactory.h"

#include <memory>
#include "IBaseScene.h"
#include "SceneChangeMediator.h"
#include "SceneID.h"

// Add more sub-scenes here...
#include "sub-scenes/LaunchingGame.h"

#include "sub-scenes/90-BackdoorMenu/BackdoorMenu.h"

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

            // Add more cases for other scenes as needed...

        default:
            return nullptr;
        }
    }
}