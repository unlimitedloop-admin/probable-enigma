#include "pch.h"

#include "GameContext.h"

#include "apps/supervisor/ResourceManager.h"
#include "config/ConfigUIManager.h"
#include "config/SoundConfig.h"
#include "core/assembly/ISnapshotProvider.h"
#include "core/assembly/ITimeController.h"
#include "core/assembly/StateProvider.h"
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

    void GameContext::AttachServices(ITimeController* time, StateProvider* input, ISnapshotProvider* snapshot) noexcept
    {
        _time = time;
        _input = input;
        _snapshot = snapshot;
    }

    bool GameContext::IsInitialized() const
    {
        return _resourceManager != nullptr && _joystickManager != nullptr;
    }
}