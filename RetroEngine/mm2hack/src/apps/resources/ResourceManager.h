//==============================================================================
// 
//  Project: mm2hack
//  ResourceManager.h
// 
//  ResourceManager class for managing game resources, accessing the ObjectManager.
// 
//==============================================================================
#pragma once

#include <optional>
#include <string>
#include "apps/rendering/bg/BGTileManager.h"
#include "apps/rendering/effects/FadeIOTexture.h"
#include "apps/rendering/fonts/FontTileManager.h"
#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/resources/bg/BGRoomBank.h"
#include "apps/systems/audio/AudioManager.h"
#include "ResourceFadeBindings.h"

namespace mm2hack::apps::resources
{
    // Manages game resources, including sprite and background tile, sound engine
    class ResourceManager
    {
        // --- type aliases (short names only inside this class) ---
        using SpriteManager = rendering::sprite::SpriteManager;
        using BGTileManager = rendering::bg::BGTileManager;
        using BGRoomBank = apps::resources::bg::BGRoomBank;
        using FontTileManager = rendering::fonts::FontTileManager;
        using AudioManager  = apps::systems::audio::AudioManager;
        using FadeIOTexture = rendering::effects::FadeIOTexture;

    public:
        ResourceManager()
            : _fades(_spriteManager, _bgTileManager, _fontTileManager, /*fallbackSpriteMax*/3, /*fallbackBGMax*/3, /*fallbackFontMax*/3) {}
        ~ResourceManager() = default;

        void UpdateEffects() { _fades.Update(); }

        void FadeOutBG(int frames, std::optional<int> to = std::nullopt) { _fades.FadeOutBG(frames, to); }
        void FadeInBG(int frames) { _fades.FadeInBG(frames); }
        void FadeOutSprite(int frames, std::optional<int> to = std::nullopt) { _fades.FadeOutSprite(frames, to); }
        void FadeInSprite(int frames) { _fades.FadeInSprite(frames); }
        void FadeOutFont(int frames, std::optional<int> to = std::nullopt) { _fades.FadeOutFont(frames, to); }
        void FadeInFont(int frames) { _fades.FadeInFont(frames); }

        // Getters
        SpriteManager& GetSpriteManager() noexcept { return _spriteManager; }
        const SpriteManager& GetSpriteManager() const noexcept { return _spriteManager; }
        BGTileManager& GetBGTileManager() noexcept { return _bgTileManager; }
        const BGTileManager& GetBGTileManager() const noexcept { return _bgTileManager; }
        BGRoomBank& GetBGRoomBank() noexcept { return _bgRoomBank; }
        const BGRoomBank& GetBGRoomBank() const noexcept { return _bgRoomBank; }
        FontTileManager& GetFontTileManager() noexcept { return _fontTileManager; }
        const FontTileManager& GetFontTileManager() const noexcept { return _fontTileManager; }
        AudioManager& GetAudioManager() noexcept { return _audioManager; }
        const AudioManager& GetAudioManager() const noexcept { return _audioManager; }

        // Releases all resources managed by the ResourceManager
        void Release();

    private:
        const std::wstring kClassName{ L"ResourceManager" };

        SpriteManager _spriteManager;       // Instance of SpriteManager
        BGTileManager _bgTileManager;       // Instance of BGTileManager
        BGRoomBank _bgRoomBank;             // Instance of BGRoomBank
        FontTileManager _fontTileManager;   // Instance of FontTileManager
        AudioManager  _audioManager;        // Instance of AudioManager

        ResourceFadeBindings _fades;        // Manages fade-in/out effects for sprites and BG tiles
    };
}