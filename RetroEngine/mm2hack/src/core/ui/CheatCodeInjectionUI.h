//==============================================================================
// 
//  Project: mm2hack
//  CheatCodeInjectionUI.h
// 
//  The UI component for managing cheat code injection.
// 
//==============================================================================
#pragma once

#include <string>
#include <vector>
#include <Windows.h>

namespace mm2hack::core::ui
{
    // A UI component for managing cheat code injection
    class CheatCodeInjectionUI
    {
    public:
        explicit CheatCodeInjectionUI(HWND parent);
        ~CheatCodeInjectionUI() {}

        // Create the controls for the cheat code injection UI
        void CreateControls();

        // Forward messages from the parent message loop (optional)
        // For example, passing WM_COMMAND / WM_NOTIFY will enable UI operations.
        bool HandleCommand(WPARAM wparam, LPARAM lparam);
        bool HandleNotify(LPARAM lparam);

        // Get the list of codes currently registered in the UI (for saving/applying)
        std::vector<std::wstring> GetCodes() const;
        std::vector<std::wstring> GetEnabledCodes() const;

        // Hooks for reading/writing existing settings (framework only)
        void LoadFromConfig();
        void SaveToConfig() const;

    private:
        // Represents a single cheat code entry in the UI
        struct CheatRow
        {
            std::wstring code;      // The code string itself (e.g., L"PAR 00A123:FF" / L"player.invincible=1 freeze")
            bool enabled{ true };   // Whether this code is enabled
            bool freeze{ false };   // Whether this code has the "freeze" option
            std::wstring type;      // "PAR" / "Symbolic" (heuristic)
            std::wstring label;     // Display label (e.g., player.invincible / 00A123)
        };

        // ---- Utilities ----
        void addFromEdit_();
        void removeSelected_();
        void clearAll_();
        void moveSelectedUp_();
        void moveSelectedDown_();
        void setAllEnabled_(bool enabled);
        void updateStatus_(const std::wstring& text) const;

        // ---- ListView ----
        void createListView_();
        void rebuildListView_();
        void insertListViewColumns_() const;
        void insertListViewRow_(size_t index, const CheatRow& row) const;
        int  getSelectedIndex_() const;
        void syncCheckStatesFromListView_(); // UI to _rows
        void syncCheckStatesToListView_();   // _rows to UI

    private:
        const std::wstring kClassName{ L"CheatCodeInjectionUI" };

        // Win32 Control IDs
        static constexpr int IDC_LABEL_CODE = 4100;
        static constexpr int IDC_EDIT_CODE = 4101;
        static constexpr int IDC_CHECK_FREEZE = 4102;
        static constexpr int IDC_BTN_ADD = 4103;
        static constexpr int IDC_BTN_REMOVE = 4104;
        static constexpr int IDC_BTN_CLEAR = 4105;
        static constexpr int IDC_BTN_ALL_ON = 4106;
        static constexpr int IDC_BTN_ALL_OFF = 4107;
        static constexpr int IDC_BTN_UP = 4108;
        static constexpr int IDC_BTN_DOWN = 4109;
        static constexpr int IDC_LIST_CODES = 4110;
        static constexpr int IDC_STATUS_TEXT = 4111;

        // Controls
        HWND _parent;
        HWND _edit_code;
        HWND _check_freeze;
        HWND _btn_add;
        HWND _btn_remove;
        HWND _btn_clear;
        HWND _btn_all_on;
        HWND _btn_all_off;
        HWND _btn_up;
        HWND _btn_down;
        HWND _list_codes;
        HWND _status_text;

        std::vector<CheatRow> _rows;    // The list of cheat rows

        // ---- Labels and Types (heuristic) ----
        static std::wstring guessType_(const std::wstring& code);   // "PAR" / "Symbolic"
        static std::wstring guessLabel_(const std::wstring& code);  // e.g., "player.invincible" / "00A123"
        static bool containsFreeze_(const std::wstring& code);      // Whether the code contains "freeze"
        static std::wstring trim_(const std::wstring& s);           // Trim whitespace from both ends
    };
}