#include "pch.h"

#include "GameContext.h"

#include <cassert>
#include "apps/resources/ResourceManager.h"
#include "config/ConfigUIManager.h"
#include "config/SoundConfig.h"
#include "core/assembly/FilteredJoystickInputProvider.h"
#include "core/diagnostics/NullWatchRegistry.h"
#include "input/JoystickManager.h"

namespace mm2hack::apps::runtime
{
    core::assembly::FilteredJoystickInputProvider& GameContext::FilteredJoystickInput() noexcept
    {
        assert(_input != nullptr);
        // NOTE: This cast is safe as long as we ensure that _input is always assigned a 'FilteredJoystickInputProvider' instance (MUST BE).
        return static_cast<FilteredJoystickInputProvider&>(*_input);
    }

    GameContext::IWatchRegistry& GameContext::Watch()
    {
        static mm2hack::core::diagnostics::NullWatchRegistry s_null;   // Fallback for uninitialized case
        return _watch ? *_watch : s_null;
    }

    void GameContext::Initialize()
    {
        using ResourceManager = apps::resources::ResourceManager;
        using JoystickManager = input::JoystickManager;

        // Set up the resources if necessary.
        _resourceManager = std::make_unique<ResourceManager>();
        _joystickManager = std::make_unique<JoystickManager>();

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
        if (_time) _time = nullptr;
        if (_input) _input = nullptr;
        if (_snapshot) _snapshot = nullptr;
        if (_watch) _watch = nullptr;
    }

    void GameContext::AttachServices(ITimeController* time, StateProvider* input, ISnapshotProvider* snapshot, IWatchRegistry* watch) noexcept
    {
        _time = time;
        _input = input;
        _snapshot = snapshot;
        _watch = watch;
    }

    bool GameContext::IsInitialized() const
    {
        return _resourceManager != nullptr && _joystickManager != nullptr;
    }
}