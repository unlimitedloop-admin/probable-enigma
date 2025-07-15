#include "WindowMessageHandlers.h"

#include <cstdio>
#include <filesystem>
#include <Windows.h>
#include "../resource.h"
#include "apps/sequence/SequenceManager.h"
#include "apps/sequence/SequenceType.h"
#include "core/GameState.h"
#include "core/GameStateManager.h"
#include "core/save/SaveData.h"
#include "core/save/SaveSystem.h"
#include "exceptions/ErrorHandler.h"
#include "exceptions/ErrorLevel.h"
#include "WindowManager.h"

namespace
{
    using namespace mm2hack::core;

    // Update the state of the save/load slot menu items based on the current save slot.
    static void UpdateSlotMenuState(HWND hWnd)
    {
        HMENU hMenu = GetMenu(hWnd);
        int selectedSlot = save::SaveSystem::GetCurrentSlot();
        for (int i = 0; i <= 9; ++i)
        {
            CheckMenuItem(hMenu, ID_SLOT_0 + i,
                MF_BYCOMMAND | ((i == selectedSlot) ? MF_CHECKED : MF_UNCHECKED));
        }
    }
}

namespace mm2hack::core::winapi
{
    void HandleCreate(HWND hWnd, LPARAM lParam)
    {
        HMENU hMenu = GetMenu(hWnd);
        CheckMenuItem(hMenu, ID_SLOT_0, MF_BYCOMMAND | MF_CHECKED); // Set the first slot as checked by default.
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
                WindowManager::GetInstance().UpdateMenuBarState();
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
            WindowManager::GetInstance().UpdateMenuBarState();
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
            WindowManager::GetInstance().UpdateMenuBarState();
            break;

        case ID_FILE_STOP:
            seq.StopCurrentSequence();
            GameStateManager::GetInstance().SetState(GameState::Standby);
            WindowManager::GetInstance().UpdateMenuBarState();
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
                exceptions::ErrorHandler::Handle(
                    L"Cannot save while the game is running.",
                    L"WindowManager",
                    L"HandleCommand",
                    exceptions::ErrorLevel::Info
                );
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
                MessageBox(hWnd, L"Failed to load game.", L"Error", MB_OK | MB_ICONERROR);
            }
            else
            {
                exceptions::ErrorHandler::Handle(
                    L"Cannot load while the game is running.",
                    L"WindowManager",
                    L"HandleCommand",
                    exceptions::ErrorLevel::Info
                );
            }
            break;

        case ID_SLOT_NEXT:
        {
            int nextSlot = (SaveSystem::GetCurrentSlot() + 1) % 10;
            SaveSystem::SetCurrentSlot(nextSlot);
            UpdateSlotMenuState(hWnd);
            break;
        }

        case ID_SLOT_PREVIOUS:
        {
            int prevSlot = (SaveSystem::GetCurrentSlot() + 9) % 10;
            SaveSystem::SetCurrentSlot(prevSlot);
            UpdateSlotMenuState(hWnd);
            break;
        }

        case ID_SLOT_EMPTY:
        {
            // Find the first empty save slot.
            for (int i = 0; i < 10; ++i)
            {
                wchar_t buffer[32];
                swprintf_s(buffer, L"sav/slot%02d.sav", i);

                if (!std::filesystem::exists(buffer))
                {
                    SaveSystem::SetCurrentSlot(i);
                    UpdateSlotMenuState(hWnd);
                    break;
                }
            }
            break;
        }

        case ID_SLOT_0: case ID_SLOT_1: case ID_SLOT_2:
        case ID_SLOT_3: case ID_SLOT_4: case ID_SLOT_5:
        case ID_SLOT_6: case ID_SLOT_7: case ID_SLOT_8: case ID_SLOT_9:
        {
            int selectedSlot = LOWORD(wParam) - ID_SLOT_0;
            SaveSystem::SetCurrentSlot(selectedSlot);
            UpdateSlotMenuState(hWnd);
            break;
        }

        case ID_SCREEN_1X:
            WindowManager::GetInstance().ChangeWindowSize(1.0f);
            break;

        case ID_SCREEN_2X:
            WindowManager::GetInstance().ChangeWindowSize(2.0f);
            break;

        case ID_SCREEN_4X:
            WindowManager::GetInstance().ChangeWindowSize(4.0f);
            break;

        case ID_SCRIPT_001:
            seq.StartTestSequence(1);
            break;

        case ID_SCRIPT_002:
            seq.StartTestSequence(2);
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
            // Enter the pause mode if the game is running and the Escape key is pressed.
            if (isFirstPress && wParam == VK_ESCAPE)
            {
                GameStateManager::GetInstance().SetState(GameState::Paused);
                WindowManager::GetInstance().UpdateMenuBarState();
                return;
            }

            return;     // If the game is running, we do not handle other key presses.
        }

        if (isFirstPress && wParam == VK_ESCAPE)
        {
            GameStateManager::GetInstance().SetState(GameState::Running);
            WindowManager::GetInstance().UpdateMenuBarState();
            return;
        }
        else if (isFirstPress && wParam == VK_F1)
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
        else if (isFirstPress && wParam == 'S')
        {
            if (GetKeyState(VK_CONTROL) & 0x8000)
            {
                // Handle Ctrl + S key press for saving the sequence.
                SendMessage(hWnd, WM_COMMAND, ID_FILE_SAVE, 0);
            }
        }
        else if (isFirstPress && wParam == 'L')
        {
            if (GetKeyState(VK_CONTROL) & 0x8000)
            {
                // Handle Ctrl + L key press for loading the sequence.
                SendMessage(hWnd, WM_COMMAND, ID_FILE_LOAD, 0);
            }
        }
        else if (isFirstPress && wParam == VK_F5)
        {
            // Handle F5 key press for toggling the menu bar.
            WindowManager::GetInstance().UpdateMenuBarState();
        }
    }

    void HandleKeyUp(HWND hWnd, WPARAM wParam)
    {

    }
}