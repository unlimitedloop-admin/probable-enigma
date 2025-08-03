#include "pch.h"

#include "WindowMessageHandlers.h"

#include <cstdio>
#include <filesystem>
#include "../resource.h"
#include "apps/deal/GameContext.h"
#include "apps/sequence/SequenceManager.h"
#include "apps/sequence/SequenceType.h"
#include "core/assembly/ScreenshotManager.h"
#include "core/GameState.h"
#include "core/GameStateManager.h"
#include "core/save/SaveData.h"
#include "core/save/SaveSystem.h"
#include "core/ui/SettingsWindow.h"
#include "exceptions/CoreException.h"
#include "exceptions/ErrorLevel.h"
#include "WindowManager.h"

// VC F12 is reserved for use by the debugger, so we use F11 in debug mode.
// For more details, refer to the following page: RegisterHotKey function (winuser.h)
// => https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerhotkey
#ifdef _DEBUG
constexpr int SCREENSHOT_KEY = VK_F11;
#else
constexpr int SCREENSHOT_KEY = VK_F12;  // Use F12 for screenshots in release mode
#endif

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

        auto setGameState = [](GameState state)
        {
            GameStateManager::GetInstance().SetState(state);
            WindowManager::GetInstance().UpdateMenuBarState();
        };

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
                setGameState(GameState::Running);
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
            setGameState(GameState::Running);
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
            setGameState(GameState::Running);
            break;

        case ID_FILE_STOP:
            seq.StopCurrentSequence();
            setGameState(GameState::Standby);
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
                THROW_EXCEPTION_EX("Cannot save while the game is running.", L"WindowManager", exceptions::ErrorLevel::Info);
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
                THROW_EXCEPTION_EX("Cannot load while the game is running.", L"WindowManager", exceptions::ErrorLevel::Info);
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

        case ID_MENU_GRAPHICS_SETTINGS:
            // Open the graphics settings window.
            overlay::SettingsWindow::OpenTab(hWnd, overlay::SettingsWindow::Tab::Graphics);
            break;

        case ID_MENU_SOUND_SETTINGS:
            // Open the sound settings window.
            overlay::SettingsWindow::OpenTab(hWnd, overlay::SettingsWindow::Tab::Sound);
            break;

        case ID_SCRIPT_001:
            seq.StartTestSequence(1);
            setGameState(GameState::Running);
            break;

        case ID_SCRIPT_002:
            seq.StartTestSequence(2);
            setGameState(GameState::Running);
            break;

        case ID_SCRIPT_003:
            seq.StartTestSequence(3);
            setGameState(GameState::Running);
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
        auto& gameContext = apps::deal::GameContext::GetInstance();

        // Handles screenshot key input with consistently high-priority execution.
        if (!(lParam & (1 << 30)) && wParam == SCREENSHOT_KEY)
        {
            assembly::ScreenshotManager::CaptureToPng();
            apps::sequence::SequenceManager::GetInstance().SendFeedback(L"Screenshot taken.");
        }

        // bit 30 = previous key state (1 = down before this message, 0 = was up before).
        const bool isFirstPress = !(lParam & (1 << 30));
        if (!isFirstPress)
        {
            return;
        }

        GameStateManager& gameState = GameStateManager::GetInstance();
        WindowManager& windowManager = WindowManager::GetInstance();

        // Pressing the ESC key immediately triggers a return and transitions the game state,
        // without interference from other key inputs.
        if (wParam == VK_ESCAPE)
        {
            if (gameState.Is(GameState::Running))
            {
                gameState.SetState(GameState::Paused);
                if (gameContext.IsInitialized())
                {
                    gameContext.GetResourceManager().GetAudioManager().Pause();
                }
            }
            else
            {
                gameState.SetState(GameState::Running);
                if (gameContext.IsInitialized())
                {
                    gameContext.GetResourceManager().GetAudioManager().Resume();
                }
            }

            windowManager.UpdateMenuBarState();
            return;
        }

        // If the game is running, we do not handle other key presses.
        if (gameState.Is(GameState::Running))
        {
            return;
        }

        switch (wParam)
        {
        case VK_F1:
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
            break;

        case 'Q':
            if ((GetKeyState(VK_CONTROL) & 0x8000) && (GetKeyState(VK_SHIFT) & 0x8000))
            {
                // Handle Ctrl + Shift + Q key press for exiting the application.
                SendMessage(hWnd, WM_COMMAND, ID_APP_EXIT, 0);
            }
            break;

        case 'R':
            if (GetKeyState(VK_CONTROL) & 0x8000)
            {
                // Handle Ctrl + R key press for reset the sequence.
                SendMessage(hWnd, WM_COMMAND, ID_FILE_RESET, 0);
            }
            break;

        case 'S':
            if (GetKeyState(VK_CONTROL) & 0x8000)
            {
                // Handle Ctrl + S key press for saving the sequence.
                SendMessage(hWnd, WM_COMMAND, ID_FILE_SAVE, 0);
            }
            break;

        case 'L':
            if (GetKeyState(VK_CONTROL) & 0x8000)
            {
                // Handle Ctrl + L key press for loading the sequence.
                SendMessage(hWnd, WM_COMMAND, ID_FILE_LOAD, 0);
            }
            break;

        case VK_F5:
            // Handle F5 key press for toggling the menu bar.
            windowManager.UpdateMenuBarState();
            break;

        default:
            break;
        }
    }

    void HandleKeyUp(HWND hWnd, WPARAM wParam)
    {

    }
}