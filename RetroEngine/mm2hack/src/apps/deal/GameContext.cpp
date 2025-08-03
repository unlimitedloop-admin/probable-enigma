#include "pch.h"

#include "GameContext.h"

#include "apps/supervisor/ResourceManager.h"
#include "config/ConfigUIManager.h"
#include "input/JoystickManager.h"

namespace mm2hack::apps::deal
{
    void GameContext::Initialize()
    {
        // Set up the resources if necessary.
        _resourceManager = std::make_unique<supervisor::ResourceManager>();
        _joystickManager = std::make_unique<input::JoystickManager>();

        using namespace config;
        SoundConfig cfg;
        ConfigUIManager::LoadSoundConfig(cfg);
        auto& audio = _resourceManager->GetAudioManager();
        audio.SetMasterVolume(cfg.master);
        audio.SetBgmVolume(cfg.bgm);
        audio.SetSeVolume(cfg.se);
        audio.SetEnabled(cfg.enabled);
    }

    void GameContext::Shutdown()
    {
        // Cleanup resources if necessary.
        if (_resourceManager)
        {
            _resourceManager->Release();
            _resourceManager.reset();
        }
        if (_joystickManager)
        {
            _joystickManager.reset();
        }
    }

    bool GameContext::IsInitialized() const
    {
        return _resourceManager != nullptr && _joystickManager != nullptr;
    }
}