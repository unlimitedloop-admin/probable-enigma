#include "WindowMessageHandlers.h"

#include <Windows.h>
#include "../resource.h"
#include "apps/sequence/SequenceManager.h"
#include "WindowManager.h"

namespace mm2hack::core::winapi
{
    void HandleCreate(HWND hWnd, LPARAM lParam)
    {

    }

    void HandleDestroy(HWND hWnd)
    {
        PostQuitMessage(0);
    }

    void HandleCommand(HWND hWnd, WPARAM wParam)
    {
        switch (LOWORD(wParam))
        {
        case ID_APP_EXIT:
            PostQuitMessage(0);
            break;
        case ID_FILE_RESET:
            apps::sequence::SequenceManager::GetInstance().RebootCurrentSequence();
            break;
        case ID_FILE_START:
            apps::sequence::SequenceManager::GetInstance().StartStandardSequence();
            break;
        case ID_FILE_START_DEBUG:
            apps::sequence::SequenceManager::GetInstance().StartDebugSequence();
            break;
        case ID_FILE_STOP:
            apps::sequence::SequenceManager::GetInstance().StopCurrentSequence();
            break;
        default:
            break;
        }
    }

    void HandlePaint(HWND hWnd)
    {

    }

    void HandleSize(HWND hWnd, WPARAM wParam, LPARAM lParam)
    {

    }

    void HandleKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
    {
        // bit 30 = previous key state (1 = down before this message, 0 = was up before)
        const bool isFirstPress = !(lParam & (1 << 30));
        if (isFirstPress && wParam == VK_F1)
        {
            if (GetKeyState(VK_SHIFT) & 0x8000)
            {
                // Handle Shift + F1 key press for starting the debug sequence
                SendMessage(hWnd, WM_COMMAND, ID_FILE_START_DEBUG, 0);
            }
            else
            {
                // Handle F1 key press for starting the standard sequence
                SendMessage(hWnd, WM_COMMAND, ID_FILE_START, 0);
            }
            return;
        }
        else if (isFirstPress && wParam == 'R')
        {
            if (GetKeyState(VK_CONTROL) & 0x8000)
            {
                SendMessage(hWnd, WM_COMMAND, ID_FILE_RESET, 0);
            }
        }
    }

    void HandleKeyUp(HWND hWnd, WPARAM wParam)
    {

    }
}