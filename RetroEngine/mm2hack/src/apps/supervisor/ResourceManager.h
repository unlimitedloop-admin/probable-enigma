//==============================================================================
// 
//  Project: mm2hack
//  ResourceManager.h
// 
//  ResourceManager class for managing game resources, accessing the ObjectManager.
// 
//==============================================================================
#pragma once

#include "apps/graphics/SpriteManager.h"

namespace mm2hack::apps::supervisor
{
    // Manages game resources, including graphics and other assets
    class ResourceManager
    {
    public:
        static ResourceManager& GetInstance()
        {
            static ResourceManager instance;
            return instance;
        }

        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;
        ResourceManager(ResourceManager&&) = delete;
        ResourceManager& operator=(ResourceManager&&) = delete;
        // ResourceManager is a singleton, so we delete the copy and move constructors and assignment operators.

        // Gets the SpriteManager instance for managing sprite textures
        graphics::SpriteManager& GetSpriteManager()
        {
            return _spriteManager;
        }

    private:
        ResourceManager() = default;
        ~ResourceManager() = default;

        graphics::SpriteManager _spriteManager;         // Instance of SpriteManager
    };
}