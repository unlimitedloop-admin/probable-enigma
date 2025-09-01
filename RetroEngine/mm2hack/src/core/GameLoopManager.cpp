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
#include "overlay/DebugHud.h"
#include "overlay/InputConfigOverlay.h"
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
        _screenHandle(context.screenHandle)
    {
        apps::deal::GameContext::GetInstance().Initialize();
        auto& jm = apps::deal::GameContext::GetInstance().GetJoystickManager();
        config::ConfigUIManager::LoadInputConfigIfMatches(jm.GetKeyBinding(), jm.ActiveDevice());
    }

    void GameLoopManager::Run()
    {
        using namespace exceptions;
        using namespace overlay;
        using namespace utils;
        using conf = config::SystemConfig;

        ScopeGuard finally([]
            {
                apps::deal::GameContext::GetInstance().Shutdown();
                apps::sequence::SequenceManager::GetInstance().Release();
            });

        auto& fps = FpsManager::GetInstance();

        try
        {
            while (!DxLib::ProcessMessage() && !DxLib::SetDrawScreen(_screenHandle) && !DxLib::ClearDrawScreen())
            {
                // If the game is paused, we skip the update logic.
                PauseManager::SetPaused(GameStateManager::GetInstance().Is(GameState::Paused));

                auto& seq = apps::sequence::SequenceManager::GetInstance();

                // Update the main sequence.
                seq.Update();
                // Render the game content.
                seq.RenderWorld();

                // Scale what we draw to fit the viewer rate.
                if (DxLib::SetDrawScreen(DX_SCREEN_BACK) ||
                    DxLib::DrawExtendGraph(0, 0,
                        static_cast<int>(conf::kScreenWidth * _viewerRate),
                        static_cast<int>(conf::kScreenHeight * _viewerRate),
                        _screenHandle, FALSE))
                {
                    break;
                }

                // Render the overlay content (e.g., HUD, debug information).
                seq.RenderOverlay();
                // If we are in JPBTN configuration mode, update the joystick manager and tick the input config overlay.
                if (GameStateManager::GetInstance().Is(GameState::JpbtnConfig))
                {
                    apps::deal::GameContext::GetInstance().GetJoystickManager().Update();
                    overlay::InputConfigOverlay::GetInstance().Tick(fps.GetDeltaSeconds());
                }
                DebugHud::GetInstance().Draw();     // Draw the FPS in the HUD, top-left corner

                // Pace & Flip the screen.
                fps.Wait();
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