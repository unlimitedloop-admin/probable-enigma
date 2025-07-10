#include "SceneFactory.h"

#include <memory>
#include "IBaseScene.h"
#include "SceneID.h"

// Add more sub-scenes here...
#include "sub-scenes/LaunchingGame.h"

namespace mm2hack::apps::scenes
{
    std::unique_ptr<IBaseScene> SceneFactory::CreateScene(SceneID id)
    {
        switch (id)
        {
        case SceneID::LaunchingGame:
            return std::make_unique<LaunchingGame>();

            // Add more cases for other scenes as needed...

        default:
            return nullptr;
        }
    }
}