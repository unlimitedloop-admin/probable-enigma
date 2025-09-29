#include "pch.h"

#include "apps/graphics/effects/FadeIOTexture.h"
#include "ResourceManager.h"

namespace mm2hack::apps::supervisor
{
    void ResourceManager::Release()
    {
        _audioManager.Release();    // Reset audio manager
    }
}