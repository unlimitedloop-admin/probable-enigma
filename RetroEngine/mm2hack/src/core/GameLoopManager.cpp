#include "pch.h"

#include "GameLoopManager.h"

#include <exception>
#include "apps/sequence/SequenceManager.h"
#include "config/ConfigUIManager.h"
#include "config/HudConfig.h"
#include "exceptions/CoreException.h"
#include "exceptions/ErrorHandler.h"
#include "exceptions/ErrorLevel.h"
#include "GameState.h"
#include "GameStateManager.h"
#include "overlay/DebugHud.h"
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
    }

    void GameLoopManager::Run()
    {
        using namespace exceptions;
        using namespace overlay;
        using namespace utils;
        using conf = config::SystemConfig;

        ScopeGuard finally([]
            {
                apps::sequence::SequenceManager::GetInstance().Release();
            });

        auto& fps = FpsManager::GetInstance();

        try
        {
            while (!DxLib::ProcessMessage() && !DxLib::SetDrawScreen(_screenHandle) && !DxLib::ClearDrawScreen())
            {
                // If the game is paused, we skip the update logic.
                PauseManager::SetPaused(GameStateManager::GetInstance().Is(GameState::Paused));

                // Update the main sequence
                apps::sequence::SequenceManager::GetInstance().Update();

                // 
                if (DxLib::SetDrawScreen(DX_SCREEN_BACK) ||
                    DxLib::DrawExtendGraph(0, 0,
                        static_cast<int>(conf::kScreenWidth * _viewerRate),
                        static_cast<int>(conf::kScreenHeight * _viewerRate),
                        _screenHandle, FALSE))
                {
                    break;
                }

                // Wait for the next frame
                fps.Wait();

                // Draw the HUD tools
                const auto& hudConfig = config::ConfigUIManager::GetCurrentHudConfig();
                if (hudConfig.showFps)
                {
                    DebugHud::GetInstance().Draw();     // Draw the FPS in the HUD, top-left corner
                }

                // Screen flip
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