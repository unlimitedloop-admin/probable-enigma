#include "pch.h"

#include "SceneChangeMediator.h"

#include "IBaseScene.h"
#include "ISceneChangedListener.h"

namespace mm2hack::apps::scenes
{
    void SceneChangeMediator::RegisterListener(ISceneChangedListener* listener)
    {
        _listener = listener;
    }

    void SceneChangeMediator::RequestChange(SceneID scene, Parameters params)
    {
        if (_listener)
        {
            _listener->RequestSceneChange(scene, params);
        }
    }
}