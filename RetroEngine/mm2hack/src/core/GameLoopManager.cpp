#include "GameLoopManager.h"

#include <DxLib.h>
#include <exception>
#include <Windows.h>
#include "apps/sequence/SequenceManager.h"
#include "exceptions/CoreException.h"
#include "exceptions/ErrorHandler.h"
#include "exceptions/ErrorLevel.h"
#include "utils/ScopeGuard.h"
#include "utils/string_converter.h"

namespace mm2hack::core
{
    GameLoopManager::GameLoopManager(HWND hWnd, const float& viewerRate)
        : _hWnd(hWnd), _viewerRate(viewerRate)
    {
    }

    void GameLoopManager::Run()
    {
        using namespace exceptions;

        utils::ScopeGuard finally([]
            {
                apps::sequence::SequenceManager::GetInstance().Release();
            });

        try
        {
            while (DxLib::ProcessMessage() == 0)
            {
                DxLib::ClearDrawScreen();
                apps::sequence::SequenceManager::GetInstance().Update();
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
                utils::utf8_to_wstring(e.what()),
                L"GameLoopManager",
                L"Run",
                ErrorLevel::FatalError
            );
        }
    }
}