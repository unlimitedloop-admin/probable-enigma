#include "CommonUIStyle.h"

#include <cstring>
#include <Windows.h>

namespace mm2hack::core::ui
{
    CommonUIStyle::CommonUIStyle()
        : _font(nullptr)
    {
        LOGFONT lf{};
        lf.lfHeight = -12;                      // 10px
        wcscpy_s(lf.lfFaceName, L"Segoe UI");   // Segoe UI(Windows 10 or later)
        lf.lfCharSet = DEFAULT_CHARSET;
        lf.lfWeight = FW_NORMAL;

        _font = CreateFontIndirect(&lf);
    }

    CommonUIStyle::~CommonUIStyle()
    {
        if (_font)
        {
            DeleteObject(_font);
            _font = nullptr;
        }
    }

    HFONT CommonUIStyle::GetUIFont() const
    {
        return _font;
    }

    void CommonUIStyle::ApplyUIFont(HWND hwnd) const
    {
        if (_font && hwnd)
        {
            SendMessage(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(_font), TRUE);
        }
    }
}