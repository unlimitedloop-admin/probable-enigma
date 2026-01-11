#include "pch.h"

#include "DebugHud.h"

#include <cstdio>
#include "apps/runtime/GameContext.h"
#include "config/ConfigUIManager.h"
#include "core/winapi/WindowManager.h"
#include "utils/decimal_decoder.h"
#include "utils/FpsManager.h"
#include "utils/string_converter.h"

namespace mm2hack::core::overlay
{
    void DebugHud::Draw() const
    {
        const auto& hud = config::ConfigUIManager::GetCurrentHudConfig();
        if (!hud.showFps && !hud.showFrameTime)
            return;

        const int x = 10;
        int y = 10;
        const int color = ::DxLib::GetColor(255, 255, 255);

        if (hud.showFrameTime)
        {
            auto& time = apps::runtime::GameContext::GetInstance().Time();
            const auto pf = time.GetPlayFrameCounter();
            wchar_t buf[64];
            if (pf == 0) {
                swprintf(buf, 64, L"PlayFrame: none");
            } else {
                swprintf(buf, 64, L"PlayFrame: %llu", static_cast<unsigned long long>(pf));
            }
            ::DxLib::DrawString(x, y, buf, color);
            y += 18;
        }

        if (hud.showFps)
        {
            auto& fps = utils::FpsManager::GetInstance();
            wchar_t buffer[64];
            if (core::winapi::WindowManager::GetInstance().IsVSyncEnabled()) {
                swprintf(buffer, 64, L"FPS: %.1f (VSync)", ::DxLib::GetFPS());
            }
            else
            {
                swprintf(buffer, 64, L"FPS: %.1f", fps.GetActualFps());
            }
            ::DxLib::DrawString(x, y, buffer, color);
            y += 18;
        }

        if (hud.showPlayerPosition)
        {
            using namespace utils;

            wchar_t buffer[64];
            ::swprintf(buffer, 128, L"PageIndex = %d", _playerPositionContext.pageIndex);
            ::DxLib::DrawString(8, y, buffer, 0xFFFFFF00);
            y += 18;
            const std::wstring xstr = decode_floating_hex_number(_playerPositionContext.x);
            const std::wstring ystr = decode_floating_hex_number(_playerPositionContext.y);
            concat_to_wchar_buffer(buffer, sizeof(buffer) / sizeof(buffer[0]), { L"Player Pos = (", xstr, L", ", ystr, L")" });
            ::DxLib::DrawString(8, y, buffer, 0xFFFF0000);
        }
    }
}