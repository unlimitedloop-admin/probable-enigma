#include "WindowMessageHandlers.h"

#include <Windows.h>
#include "../resource.h"
#include "apps/sequence/SequenceManager.h"
#include "apps/sequence/SequenceType.h"
#include "core/GameState.h"
#include "core/GameStateManager.h"
#include "core/save/SaveData.h"
#include "core/save/SaveSystem.h"
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
        using namespace save;
        using namespace apps::sequence;
        auto& seq = SequenceManager::GetInstance();

        auto shouldConfirmReboot = [&]() -> bool
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
            if (seq.GetCurrentSequenceType() != SequenceType::None)
            {
                // NOTE: Even if a reset command is issued while paused, the system is designed to resume operation.
                GameStateManager::GetInstance().SetState(GameState::Running);
                WindowManager::GetInstance().UpdateMenuBarActivate();
            }
            break;

        case ID_FILE_START:
            if (seq.GetCurrentSequenceType() == SequenceType::Standard)
            {
                if (shouldConfirmReboot())
                {
                    break;
                }
            }
            seq.StartStandardSequence();
            GameStateManager::GetInstance().SetState(GameState::Running);
            WindowManager::GetInstance().UpdateMenuBarActivate();
            break;

        case ID_FILE_START_DEBUG:
            if (seq.GetCurrentSequenceType() == SequenceType::Debug)
            {
                if (shouldConfirmReboot())
                {
                    break;
                }
            }
            seq.StartDebugSequence();
            GameStateManager::GetInstance().SetState(GameState::Running);
            WindowManager::GetInstance().UpdateMenuBarActivate();
            break;

        case ID_FILE_STOP:
            seq.StopCurrentSequence();
            GameStateManager::GetInstance().SetState(GameState::Standby);
            WindowManager::GetInstance().UpdateMenuBarActivate();
            break;

        case ID_FILE_SAVE:
            if (GameStateManager::GetInstance().Is(GameState::Paused))
            {
                SaveData data{};
                const auto path = SaveSystem::GetCurrentSlotFilename();
                if (auto* sequence = seq.GetCurrentSequence(); sequence != nullptr)
                {
                    if (sequence->Save(data) && SaveSystem::Save(path, data))
                    {
                        MessageBox(hWnd, (L"Saved to " + path).c_str(), L"Save", MB_OK);
                        break;
                    }
                }
                MessageBox(hWnd, L"Failed to save game.", L"Error", MB_ICONERROR);
            }
            else
            {
                MessageBox(hWnd, L"Saving is only allowed when the game is running and paused.", L"Warning", MB_ICONWARNING);
            }
            break;

        case ID_FILE_LOAD:
            if (GameStateManager::GetInstance().Is(GameState::Paused))
            {
                SaveData data{};
                const auto path = SaveSystem::GetCurrentSlotFilename();
                if (SaveSystem::Load(path, data))
                {
                    seq.LoadSequence(static_cast<SequenceType>(data.sequenceID));
                    if (auto* sequence = seq.GetCurrentSequence(); sequence != nullptr)
                    {
                        if (sequence->Load(data))
                        {
                            MessageBox(hWnd, (L"Loaded from " + path).c_str(), L"Load", MB_OK);
                            break;
                        }
                    }
                }
                MessageBox(hWnd, L"Failed to load game.", L"Error", MB_ICONERROR);
            }
            else
            {
                MessageBox(hWnd, L"Load is only allowed during pause.", L"Warning", MB_ICONWARNING);
            }
            break;

        case ID_SCRIPT_001:
            seq.StartTestSequence(1);
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
        // bit 30 = previous key state (1 = down before this message, 0 = was up before).
        const bool isFirstPress = !(lParam & (1 << 30));

        if (GameStateManager::GetInstance().Is(GameState::Running))
        {
            if (isFirstPress && wParam == VK_ESCAPE)
            {
                GameStateManager::GetInstance().SetState(GameState::Paused);
                WindowManager::GetInstance().UpdateMenuBarActivate();
                return;
            }
        }

        if (isFirstPress && wParam == VK_F1)
        {
            if (GetKeyState(VK_SHIFT) & 0x8000)
            {
                // Handle Shift + F1 key press for starting the debug sequence.
                SendMessage(hWnd, WM_COMMAND, ID_FILE_START_DEBUG, 0);
            }
            else
            {
                // Handle F1 key press for starting the standard sequence.
                SendMessage(hWnd, WM_COMMAND, ID_FILE_START, 0);
            }
            return;
        }
        else if (isFirstPress && wParam == 'R')
        {
            if (GetKeyState(VK_CONTROL) & 0x8000)
            {
                // Handle Ctrl + R key press for reset the sequence.
                SendMessage(hWnd, WM_COMMAND, ID_FILE_RESET, 0);
            }
        }
    }

    void HandleKeyUp(HWND hWnd, WPARAM wParam)
    {

    }
}