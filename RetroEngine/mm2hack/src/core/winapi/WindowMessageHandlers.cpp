#include "WindowMessageHandlers.h"

#include <Windows.h>
#include "../resource.h"
#include "apps/sequence/SequenceManager.h"
#include "apps/sequence/SequenceType.h"

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
        using namespace apps::sequence;
        auto& seq = SequenceManager::GetInstance();

        auto shouldConfirmReboot = [&](SequenceType type) -> bool
            {
                int result = MessageBox(
                    hWnd,
                    L"Reboot sequence?",
                    L"mm2hack",
                    MB_OKCANCEL | MB_ICONQUESTION | MB_DEFBUTTON2
                );
                return result == IDCANCEL;
            };

        switch (LOWORD(wParam))
        {
        case ID_APP_EXIT:
            PostQuitMessage(0);
            break;

        case ID_FILE_RESET:
            seq.RebootCurrentSequence();
            break;

        case ID_FILE_START:
            if (seq.GetCurrentSequenceType() == SequenceType::Standard)
            {
                if (shouldConfirmReboot(SequenceType::Standard))
                {
                    break;
                }
            }
            seq.StartStandardSequence();
            break;

        case ID_FILE_START_DEBUG:
            if (seq.GetCurrentSequenceType() == SequenceType::Debug)
            {
                if (shouldConfirmReboot(SequenceType::Debug))
                {
                    break;
                }
            }
            seq.StartDebugSequence();
            break;

        case ID_FILE_STOP:
            seq.StopCurrentSequence();
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