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
#include "apps/audio/AudioManager.h"
#include "apps/graphics/bg/BGRoomBank.h"
#include "apps/graphics/bg/BGTileManager.h"
#include "apps/graphics/effects/FadeIOTexture.h"
#include "apps/graphics/fonts/FontTileManager.h"
#include "apps/graphics/sprite/SpriteManager.h"
#include "apps/supervisor/ResourceFadeBindings.h"

namespace mm2hack::apps::supervisor
{
    // Manages game resources, including sprite and background tile, sound engine
    class ResourceManager
    {
    public:
        // --- type aliases (short names only inside this class) ---
        using SpriteManager = graphics::sprite::SpriteManager;
        using BGTileManager = graphics::bg::BGTileManager;
        using BGRoomBank = graphics::bg::BGRoomBank;
        using FontTileManager = graphics::fonts::FontTileManager;
        using AudioManager  = audio::AudioManager;
        using FadeIOTexture = graphics::effects::FadeIOTexture;

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
        SpriteManager _spriteManager;       // Instance of SpriteManager
        BGTileManager _bgTileManager;       // Instance of BGTileManager
        BGRoomBank _bgRoomBank;             // Instance of BGRoomBank
        FontTileManager _fontTileManager;   // Instance of FontTileManager
        AudioManager  _audioManager;        // Instance of AudioManager

        ResourceFadeBindings _fades;        // Manages fade-in/out effects for sprites and BG tiles
    };
}