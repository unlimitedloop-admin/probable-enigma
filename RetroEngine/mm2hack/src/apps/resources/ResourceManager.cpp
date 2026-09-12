#include "pch.h"

#include "ResourceManager.h"

namespace mm2hack::apps::resources
{
    void ResourceManager::Release()
    {
        _audioManager.Release();    // Reset audio manager
    }
}