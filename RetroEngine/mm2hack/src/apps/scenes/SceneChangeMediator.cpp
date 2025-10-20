#include "pch.h"

#include "SceneChangeMediator.h"

#include "apps/parameters/Parameters.h"
#include "ISceneChangedListener.h"
#include "SceneID.h"

namespace mm2hack::apps::scenes
{
    void SceneChangeMediator::RegisterListener(ISceneChangedListener* listener)
    {
        _listener = listener;
    }

    void SceneChangeMediator::RequestChange(SceneID scene, parameters::Parameters params)
    {
        if (_listener)
        {
            _listener->RequestSceneChange(scene, params);
        }
    }
}