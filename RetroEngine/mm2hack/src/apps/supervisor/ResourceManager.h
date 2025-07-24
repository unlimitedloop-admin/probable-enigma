//==============================================================================
// 
//  Project: mm2hack
//  ResourceManager.h
// 
//  ResourceManager class for managing game resources, accessing the ObjectManager.
// 
//==============================================================================
#pragma once

#include "apps/graphics/BGTileManager.h"
#include "apps/graphics/SpriteManager.h"

namespace mm2hack::apps::supervisor
{
    // Manages game resources, including sprite and background tile, sound engine
    class ResourceManager
    {
    public:
        ResourceManager() = default;
        ~ResourceManager() = default;

        // Gets the SpriteManager instance for managing sprite textures
        graphics::SpriteManager& GetSpriteManager() { return _spriteManager; }
        // Gets the BGTileManager instance for managing background tile textures
        graphics::BGTileManager& GetBGTileManager() { return _bgTileManager; }

    private:
        graphics::SpriteManager _spriteManager;         // Instance of SpriteManager
        graphics::BGTileManager _bgTileManager;         // Instance of BGTileManager
    };
}