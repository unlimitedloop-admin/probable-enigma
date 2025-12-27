#include "pch.h"

#include "CheatCodeInjectionUI.h"

#include <cctype>
#include <libloaderapi.h>
#include <Windowsx.h>
#include "CommonUIStyle.h"

namespace
{
    HINSTANCE GetHinst() { return GetModuleHandle(nullptr); }

    // Trim whitespace from both ends of a wide string
    std::wstring GetWindowTextWString(HWND h)
    {
        const int len = GetWindowTextLength(h);
        std::wstring s(len, L'\0');
        if (len > 0)
        {
            GetWindowText(h, s.data(), len + 1);
        }
        return s;
    }
}

namespace mm2hack::core::ui
{
    CheatCodeInjectionUI::CheatCodeInjectionUI(HWND parent)
        : _parent(parent),
        _edit_code(nullptr),
        _check_freeze(nullptr),
        _btn_add(nullptr),
        _btn_remove(nullptr),
        _btn_clear(nullptr),
        _btn_all_on(nullptr),
        _btn_all_off(nullptr),
        _btn_up(nullptr),
        _btn_down(nullptr),
        _list_codes(nullptr),
        _status_text(nullptr)
    {
    }

    void CheatCodeInjectionUI::CreateControls()
    {
        ui::CommonUIStyle uiStyle;

        // --- Input Buttons ---
        auto label = CreateWindowEx(0, L"STATIC", L"Cheat Code:",
            WS_CHILD | WS_VISIBLE,
            20, 20, 90, 20, _parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_LABEL_CODE)),
            GetHinst(), nullptr);
        uiStyle.ApplyUIFont(label);

        _edit_code = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            115, 18, 360, 22, _parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_EDIT_CODE)),
            GetHinst(), nullptr);
        uiStyle.ApplyUIFont(_edit_code);

        _check_freeze = CreateWindowEx(0, L"BUTTON", L"Add Freeze",
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            485, 18, 120, 22, _parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_CHECK_FREEZE)),
            GetHinst(), nullptr);
        uiStyle.ApplyUIFont(_check_freeze);

        _btn_add = CreateWindowEx(0, L"BUTTON", L"Add",
            WS_CHILD | WS_VISIBLE,
            620, 18, 60, 22, _parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_ADD)),
            GetHinst(), nullptr);
        uiStyle.ApplyUIFont(_btn_add);

        _btn_remove = CreateWindowEx(0, L"BUTTON", L"Remove",
            WS_CHILD | WS_VISIBLE,
            690, 18, 60, 22, _parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_REMOVE)),
            GetHinst(), nullptr);
        uiStyle.ApplyUIFont(_btn_remove);

        _btn_clear = CreateWindowEx(0, L"BUTTON", L"Clear All",
            WS_CHILD | WS_VISIBLE,
            760, 18, 70, 22, _parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_CLEAR)),
            GetHinst(), nullptr);
        uiStyle.ApplyUIFont(_btn_clear);

        // --- Batch Operations ---
        _btn_all_on = CreateWindowEx(0, L"BUTTON", L"Enable All",
            WS_CHILD | WS_VISIBLE,
            20, 56, 80, 22, _parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_ALL_ON)),
            GetHinst(), nullptr);
        uiStyle.ApplyUIFont(_btn_all_on);

        _btn_all_off = CreateWindowEx(0, L"BUTTON", L"Disable All",
            WS_CHILD | WS_VISIBLE,
            110, 56, 80, 22, _parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_ALL_OFF)),
            GetHinst(), nullptr);
        uiStyle.ApplyUIFont(_btn_all_off);

        _btn_up = CreateWindowEx(0, L"BUTTON", L"Up",
            WS_CHILD | WS_VISIBLE,
            200, 56, 80, 22, _parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_UP)),
            GetHinst(), nullptr);
        uiStyle.ApplyUIFont(_btn_up);

        _btn_down = CreateWindowEx(0, L"BUTTON", L"Down",
            WS_CHILD | WS_VISIBLE,
            290, 56, 80, 22, _parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BTN_DOWN)),
            GetHinst(), nullptr);
        uiStyle.ApplyUIFont(_btn_down);

        // --- Cheater listView ---
        createListView_();

        // --- Show status ---
        _status_text = CreateWindowEx(0, L"STATIC", L"Ready.",
            WS_CHILD | WS_VISIBLE,
            20, 430, 810, 20, _parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_STATUS_TEXT)),
            GetHinst(), nullptr);
        uiStyle.ApplyUIFont(_status_text);

        LoadFromConfig();
        rebuildListView_();
    }

    bool CheatCodeInjectionUI::HandleCommand(WPARAM wparam, LPARAM /*lparam*/)
    {
        const int id = LOWORD(wparam);
        const int code = HIWORD(wparam);

        if (code == BN_CLICKED)
        {
            switch (id)
            {
            case IDC_BTN_ADD:
                addFromEdit_(); return true;
            case IDC_BTN_REMOVE:
                removeSelected_(); return true;
            case IDC_BTN_CLEAR:
                clearAll_(); return true;
            case IDC_BTN_ALL_ON:
                setAllEnabled_(true); return true;
            case IDC_BTN_ALL_OFF:
                setAllEnabled_(false); return true;
            case IDC_BTN_UP:
                moveSelectedUp_(); return true;
            case IDC_BTN_DOWN:
                moveSelectedDown_(); return true;
            default:
                break;
            }
        }
        return false;
    }

    bool CheatCodeInjectionUI::HandleNotify(LPARAM lparam)
    {
        auto* hdr = reinterpret_cast<NMHDR*>(lparam);
        if (hdr && hdr->hwndFrom == _list_codes)
        {
            switch (hdr->code)
            {
            case LVN_ITEMCHANGED:
                syncCheckStatesFromListView_();
                return true;
            default:
                break;
            }
        }
        return false;
    }

    std::vector<std::wstring> CheatCodeInjectionUI::GetCodes() const
    {
        std::vector<std::wstring> out;
        out.reserve(_rows.size());
        for (const auto& r : _rows) { out.push_back(r.code); }
        return out;
    }

    std::vector<std::wstring> CheatCodeInjectionUI::GetEnabledCodes() const
    {
        std::vector<std::wstring> out;
        for (const auto& r : _rows)
        {
            if (r.enabled) { out.push_back(r.code); }
        }
        return out;
    }

    void CheatCodeInjectionUI::LoadFromConfig()
    {
        // HACK: Construct _rows from INI/JSON
        // Ex: _rows = config::ConfigUIManager::LoadCheatCodes();
    }

    void CheatCodeInjectionUI::SaveToConfig() const
    {
        // HACK: Save for current _rows to INI/JSON
        // Ex: config::ConfigUIManager::SaveCheatCodes(_rows);
    }

    void CheatCodeInjectionUI::createListView_()
    {
        ui::CommonUIStyle uiStyle;

        _list_codes = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
            WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL,
            20, 90, 810, 330, _parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_LIST_CODES)),
            GetHinst(), nullptr);
        uiStyle.ApplyUIFont(_list_codes);

        ListView_SetExtendedListViewStyle(_list_codes,
            LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES | LVS_EX_LABELTIP);

        insertListViewColumns_();
    }

    void CheatCodeInjectionUI::insertListViewColumns_() const
    {
        LVCOLUMNW col{};
        col.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

        col.cx = 50;  col.pszText = const_cast<wchar_t*>(L"Enabled");   col.iSubItem = 0;
        ListView_InsertColumn(_list_codes, 0, &col);

        col.cx = 220; col.pszText = const_cast<wchar_t*>(L"Label/Code"); col.iSubItem = 1;
        ListView_InsertColumn(_list_codes, 1, &col);

        col.cx = 90;  col.pszText = const_cast<wchar_t*>(L"Type"); col.iSubItem = 2;
        ListView_InsertColumn(_list_codes, 2, &col);

        col.cx = 70;  col.pszText = const_cast<wchar_t*>(L"Freeze"); col.iSubItem = 3;
        ListView_InsertColumn(_list_codes, 3, &col);

        col.cx = 360; col.pszText = const_cast<wchar_t*>(L"Code String"); col.iSubItem = 4;
        ListView_InsertColumn(_list_codes, 4, &col);
    }

    void CheatCodeInjectionUI::rebuildListView_()
    {
        ListView_DeleteAllItems(_list_codes);

        for (size_t i = 0; i < _rows.size(); ++i)
        {
            insertListViewRow_(i, _rows[i]);
        }
        syncCheckStatesToListView_();
    }

    void CheatCodeInjectionUI::insertListViewRow_(size_t index, const CheatRow& row) const
    {
        LVITEMW item{};
        item.mask = LVIF_TEXT;
        item.iItem = static_cast<int>(index);
        item.iSubItem = 1;
        std::wstring label = row.label.empty() ? row.code : row.label;
        item.pszText = const_cast<wchar_t*>(label.c_str());
        int idx = ListView_InsertItem(_list_codes, &item);

        ListView_SetItemText(_list_codes, idx, 2, const_cast<wchar_t*>(row.type.c_str()));
        ListView_SetItemText(_list_codes, idx, 3, const_cast<wchar_t*>(row.freeze ? L"Yes" : L"No"));
        ListView_SetItemText(_list_codes, idx, 4, const_cast<wchar_t*>(row.code.c_str()));
    }

    int CheatCodeInjectionUI::getSelectedIndex_() const
    {
        return ListView_GetNextItem(_list_codes, -1, LVNI_SELECTED);
    }

    void CheatCodeInjectionUI::syncCheckStatesFromListView_()
    {
        for (int i = 0; i < static_cast<int>(_rows.size()); ++i)
        {
            _rows[static_cast<size_t>(i)].enabled = ListView_GetCheckState(_list_codes, i) != FALSE;
        }
    }

    void CheatCodeInjectionUI::syncCheckStatesToListView_()
    {
        for (int i = 0; i < static_cast<int>(_rows.size()); ++i)
        {
            ListView_SetCheckState(_list_codes, i, _rows[static_cast<size_t>(i)].enabled ? TRUE : FALSE);
        }
    }

    void CheatCodeInjectionUI::addFromEdit_()
    {
        std::wstring code = trim_(GetWindowTextWString(_edit_code));
        if (code.empty())
        {
            updateStatus_(L"Cannot add empty code.");
            return;
        }

        // Add "freeze" if the checkbox is checked and not already present.
        if (Button_GetCheck(_check_freeze) && !containsFreeze_(code))
        {
            code += L" freeze";
        }

        CheatRow row{};
        row.code = code;
        row.type = guessType_(code);
        row.freeze = containsFreeze_(code);
        row.label = guessLabel_(code);
        row.enabled = true;

        _rows.push_back(std::move(row));
        rebuildListView_();
        updateStatus_(L"Added successfully.");

        SetWindowText(_edit_code, L"");
        SetFocus(_edit_code);
    }

    void CheatCodeInjectionUI::removeSelected_()
    {
        const int sel = getSelectedIndex_();
        if (sel < 0)
        {
            updateStatus_(L"Cannot remove: No item selected.");
            return;
        }
        _rows.erase(_rows.begin() + sel);
        rebuildListView_();
        updateStatus_(L"Removed successfully.");
    }

    void CheatCodeInjectionUI::clearAll_()
    {
        _rows.clear();
        rebuildListView_();
        updateStatus_(L"Cleared all.");
    }

    void CheatCodeInjectionUI::moveSelectedUp_()
    {
        const int sel = getSelectedIndex_();
        if (sel <= 0) { return; }
        std::swap(_rows[sel], _rows[sel - 1]);
        rebuildListView_();
        ListView_SetItemState(_list_codes, sel - 1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    }

    void CheatCodeInjectionUI::moveSelectedDown_()
    {
        const int sel = getSelectedIndex_();
        if (sel < 0 || sel >= static_cast<int>(_rows.size()) - 1) { return; }
        std::swap(_rows[sel], _rows[sel + 1]);
        rebuildListView_();
        ListView_SetItemState(_list_codes, sel + 1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    }

    void CheatCodeInjectionUI::setAllEnabled_(bool enabled)
    {
        for (auto& r : _rows) { r.enabled = enabled; }
        syncCheckStatesToListView_();
        updateStatus_(enabled ? L"Enabled all." : L"Disabled all.");
    }

    void CheatCodeInjectionUI::updateStatus_(const std::wstring& text) const
    {
        SetWindowText(_status_text, text.c_str());
    }

    // ---- String Utilities (frame)---------------------------------------------
    std::wstring CheatCodeInjectionUI::guessType_(const std::wstring& code)
    {
        // Rules: Input starting with "PAR" is "PAR", otherwise "Symbolic".
        if (code.size() >= 3)
        {
            if (code.rfind(L"PAR", 0) == 0) { return L"PAR"; }
        }
        return L"Symbolic";
    }

    std::wstring CheatCodeInjectionUI::guessLabel_(const std::wstring& code)
    {
        // Find "player.invincible=1 freeze" for get "player.invincible".
        auto posEq = code.find(L'=');
        if (posEq != std::wstring::npos)
        {
            return trim_(code.substr(0, posEq));
        }
        // PAR 00A123:FF for get 00A123
        auto posPar = code.find(L"PAR ");
        auto posColon = code.find(L':');
        if (posPar == 0 && posColon != std::wstring::npos && posColon > 4)
        {
            return trim_(code.substr(4, posColon - 4));
        }
        return L"";
    }

    bool CheatCodeInjectionUI::containsFreeze_(const std::wstring& code)
    {
        return code.find(L"freeze") != std::wstring::npos || code.find(L"FREEZE") != std::wstring::npos;
    }

    std::wstring CheatCodeInjectionUI::trim_(const std::wstring& s)
    {
        size_t a = 0, b = s.size();
        while (a < b && iswspace(s[a])) ++a;
        while (b > a && iswspace(s[b - 1])) --b;
        return s.substr(a, b - a);
    }
}