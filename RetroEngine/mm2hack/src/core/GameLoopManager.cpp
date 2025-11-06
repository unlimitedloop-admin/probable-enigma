#include "pch.h"

#include "GameLoopManager.h"

#include <exception>
#include "apps/runtime/GameContext.h"
#include "apps/sequence/SequenceManager.h"
#include "assembly/ISnapshotProvider.h"
#include "assembly/JoystickInputProviderAdapter.h"
#include "assembly/StandardTimeController.h"
#include "config/ConfigUIManager.h"
#include "GameState.h"
#include "GameStateManager.h"
#include "overlay/PauseManager.h"
#include "utils/Fps.h"
#include "utils/FpsManager.h"
#include "utils/ScopeGuard.h"
#include "utils/string_converter.h"
#include "winapi/WindowContext.h"

namespace mm2hack::core
{
    GameLoopManager::GameLoopManager(winapi::WindowContext& context)
        : _hWnd(context.hWnd),
        _viewerRate(context.viewerRate),
        _screenHandle(context.screenHandle),
        _vSync(context.vSync)
    {
        using namespace assembly;
        using namespace config;

        // Set up time controller.
        auto& fps = utils::FpsManager::GetInstance();
        _time = std::make_unique<StandardTimeController>(nullptr, false);
        _time->EnableFollowFps(false);  // Disable follow FPS by default.

        // Initialize game context and load input-device(joycard) configuration.
        auto& gcInstance = apps::runtime::GameContext::GetInstance();
        gcInstance.Initialize();
        auto& jm = gcInstance.Joystick();
        ConfigUIManager::LoadInputConfigIfMatches(jm.GetKeyBinding(), jm.ActiveDevice());

        // Take over to the GameContext services (time controller, input state provider, snapshot provider).
        auto time = std::make_unique<StandardTimeController>(/*fps=*/nullptr, /*callWait=*/false);
        auto input = std::make_unique<JoystickInputProviderAdapter>(jm);
        ISnapshotProvider* snapshot = nullptr;
        gcInstance.AttachServices(time.get(), input.get(), snapshot);

        // Move ownership to member variables.
        _time = std::move(time);
        _input = std::move(input);
    }

    void GameLoopManager::Run()
    {
        using namespace config;
        using namespace exceptions;
        using namespace overlay;
        using namespace utils;

        ScopeGuard finally([]
            {
                auto& seq = apps::sequence::SequenceManager::GetInstance();
                seq.Release();
                auto& ctx = apps::runtime::GameContext::GetInstance();
                ctx.Shutdown();
            });

        // Load graphics configuration and apply FPS limit if changed.
        auto& fps = FpsManager::GetInstance();
        auto& seq = apps::sequence::SequenceManager::GetInstance();

        try
        {
            while (!::DxLib::ProcessMessage() && !::DxLib::SetDrawScreen(_screenHandle) && !::DxLib::ClearDrawScreen())
            {
                _time->BeginFrame();    // Defines delta time for this frame.

                auto destW = static_cast<int>(SystemConfig::kScreenWidth * _viewerRate);
                auto destH = static_cast<int>(SystemConfig::kScreenHeight * _viewerRate);

                // If the game is paused, we skip the update logic.
                PauseManager::SetPaused(GameStateManager::GetInstance().Is(GameState::Paused));

                // --- Go game update & rendering ---
                seq.Update();                                   // Update the main sequence.
                seq.RenderWorld(_screenHandle, destW, destH);   // Render the game content, and extended rate scaling.
                seq.RenderOverlay(destW, destH);                // Render the overlay content (e.g., HUD, debug information).

                // Render the input configuration overlay if active.
                seq.HandleJpbtnConfigMode(static_cast<double>(_time->DeltaSeconds()));

                // Pace & Flip the screen.
                fps.Wait();
                // VSync control = pseudo FPS with monitor refresh rate.
                if (_vSync) ::DxLib::WaitVSync(1);
                // Screen flip to present the rendered frame of the back buffer.
                ::DxLib::ScreenFlip();

                _time->EndFrame();      // End of frame processing.
            }
        }
        catch (const CoreException& ex)
        {
            ErrorHandler::HandleEx(ex);
        }
        catch (const std::exception& e)
        {
            ErrorHandler::Handle(utf8_to_wstring(e.what()), kClassName, L"Run", ErrorLevel::FatalError);
        }
    }
}