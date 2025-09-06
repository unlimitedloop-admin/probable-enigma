//==============================================================================
// 
//  Project: mm2hack
//  CheatCodeInjectionUI.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include <string>
#include <vector>
#include <Windows.h>

namespace mm2hack::core::ui
{
    class CheatCodeInjectionUI
    {
    public:
        explicit CheatCodeInjectionUI(HWND parent);
        ~CheatCodeInjectionUI() {}

        void CreateControls();

        // Forward messages from the parent message loop (optional)
        // For example, passing WM_COMMAND / WM_NOTIFY will enable UI operations.
        bool HandleCommand(WPARAM wparam, LPARAM lparam);
        bool HandleNotify(LPARAM lparam);

        // Get the list of codes currently registered in the UI (for saving/applying)
        std::vector<std::wstring> GetCodes() const;
        std::vector<std::wstring> GetEnabledCodes() const;

        // Hooks for reading/writing existing settings (framework only)
        void LoadFromConfig();     // TODO: Work In Progress
        void SaveToConfig() const; // TODO: Work In Progress

    private:
        struct CheatRow
        {
            std::wstring code;      // The code string itself (e.g., L"PAR 00A123:FF" / L"player.invincible=1 freeze")
            bool enabled{ true };
            bool freeze{ false };
            std::wstring type;      // "PAR" / "Symbolic" (heuristic)
            std::wstring label;     // Display label (e.g., player.invincible / 00A123)
        };

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

        std::vector<CheatRow> _rows;

    private:
        // Utilities
        void AddFromEdit();
        void RemoveSelected();
        void ClearAll();
        void MoveSelectedUp();
        void MoveSelectedDown();
        void SetAllEnabled(bool enabled);
        void UpdateStatus(const std::wstring& text) const;

        // ListView
        void CreateListView();
        void RebuildListView();
        void InsertListViewColumns() const;
        void InsertListViewRow(size_t index, const CheatRow& row) const;
        int  GetSelectedIndex() const;
        void SyncCheckStatesFromListView(); // UI to _rows
        void SyncCheckStatesToListView();   // _rows to UI

        // Labels and Types (heuristic)
        static std::wstring GuessType(const std::wstring& code);
        static std::wstring GuessLabel(const std::wstring& code);
        static bool ContainsFreeze(const std::wstring& code);
        static std::wstring Trim(const std::wstring& s);
    };
}