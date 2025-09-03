#include "pch.h"

#include "GameLoopManager.h"

#include <exception>
#include "apps/deal/GameContext.h"
#include "apps/sequence/SequenceManager.h"
#include "config/ConfigUIManager.h"
#include "exceptions/CoreException.h"
#include "exceptions/ErrorHandler.h"
#include "exceptions/ErrorLevel.h"
#include "GameState.h"
#include "GameStateManager.h"
#include "overlay/PauseManager.h"
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
        auto& gcInstance = apps::deal::GameContext::GetInstance();
        gcInstance.Initialize();
        auto& jm = gcInstance.GetJoystickManager();
        config::ConfigUIManager::LoadInputConfigIfMatches(jm.GetKeyBinding(), jm.ActiveDevice());
    }

    void GameLoopManager::Run()
    {
        using namespace config;
        using namespace exceptions;
        using namespace overlay;
        using namespace utils;

        ScopeGuard finally([]
            {
                apps::deal::GameContext::GetInstance().Shutdown();
                apps::sequence::SequenceManager::GetInstance().Release();
            });

        // Load graphics configuration and apply FPS limit if changed.
        auto& fps = FpsManager::GetInstance();

        try
        {
            while (!DxLib::ProcessMessage() && !DxLib::SetDrawScreen(_screenHandle) && !DxLib::ClearDrawScreen())
            {
                auto& seq = apps::sequence::SequenceManager::GetInstance();

                // If the game is paused, we skip the update logic.
                PauseManager::SetPaused(GameStateManager::GetInstance().Is(GameState::Paused));

                // Update the main sequence.
                seq.Update();
                // Render the game content.
                seq.RenderWorld(_screenHandle, _viewerRate);        // Render of game contents.
                // Render the overlay content (e.g., HUD, debug information).
                seq.RenderOverlay(_viewerRate);                     // Render of overlay contents.
                // Render the input configuration overlay if active.
                seq.HandleJpbtnConfigMode(fps.GetDeltaSeconds());
                // Pace & Flip the screen.
                fps.Wait();

                if (_vSync) DxLib::WaitVSync(1);  // VSync control = pseudo FPS with monitor refresh rate.
                // Screen flip to present the rendered frame of the back buffer.
                DxLib::ScreenFlip();
            }
        }
        catch (const CoreException& ex)
        {
            ErrorHandler::HandleEx(ex);
        }
        catch (const std::exception& e)
        {
            ErrorHandler::Handle(
                utf8_to_wstring(e.what()),
                L"GameLoopManager",
                L"Run",
                ErrorLevel::FatalError
            );
        }
    }
}