#include "GameLoopManager.h"

#include <cstdint>
#include <DxLib.h>
#include <Windows.h>
#include "apps/sequence/SequenceManager.h"

namespace mm2hack::core
{
    GameLoopManager::GameLoopManager(HWND hWnd, const float& viewerRate)
        : _hWnd(hWnd), _viewerRate(viewerRate)
    {
    }

    void GameLoopManager::Run()
    {
        while (DxLib::ProcessMessage() == 0)
        {
            DxLib::ClearDrawScreen();
            apps::sequence::SequenceManager::GetInstance().Update();
            DxLib::ScreenFlip();
        }
    }
}