//==============================================================================
// 
//  Project: mm2hack
//  ResourceManager.h
// 
//  ResourceManager class for managing game resources, accessing the ObjectManager.
// 
//==============================================================================
#pragma once

#include "apps/audio/AudioManager.h"
#include "apps/graphics/bg/BGTileManager.h"
#include "apps/graphics/sprite/SpriteManager.h"

namespace mm2hack::apps::supervisor
{
    // Manages game resources, including sprite and background tile, sound engine
    class ResourceManager
    {
    public:
        // --- type aliases (short names only inside this class) ---
        using SpriteManager = graphics::sprite::SpriteManager;
        using BGTileManager = graphics::bg::BGTileManager;
        using AudioManager  = audio::AudioManager;

        ResourceManager() = default;
        ~ResourceManager() = default;

        // Getters
        SpriteManager& GetSpriteManager() noexcept { return _spriteManager; }
        const SpriteManager& GetSpriteManager() const noexcept { return _spriteManager; }

        BGTileManager& GetBGTileManager() noexcept { return _bgTileManager; }
        const BGTileManager& GetBGTileManager() const noexcept { return _bgTileManager; }

        AudioManager& GetAudioManager() noexcept { return _audioManager; }
        const AudioManager& GetAudioManager() const noexcept { return _audioManager; }

        // Releases all resources managed by the ResourceManager
        void Release();

    private:
        SpriteManager _spriteManager; // Instance of SpriteManager
        BGTileManager _bgTileManager; // Instance of BGTileManager
        AudioManager  _audioManager;  // Instance of AudioManager
    };
}