#include "pch.h"

#include "DebugHud.h"

#include <cstdio>
#include "apps/runtime/GameContext.h"
#include "config/ConfigUIManager.h"
#include "utils/FpsManager.h"

namespace mm2hack::core::overlay
{
    void DebugHud::Draw() const
    {
        const auto& hud = config::ConfigUIManager::GetCurrentHudConfig();
        if (!hud.showFps && !hud.showFrameTime)
            return;

        const int x = 10;
        int y = 10;
        const int color = DxLib::GetColor(255, 255, 255);

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
            DxLib::DrawString(x, y, buf, color);
            y += 18;
        }

        if (hud.showFps)
        {
            auto& fps = utils::FpsManager::GetInstance();
            TCHAR buffer[64];
            swprintf(buffer, 64, L"FPS: %.1f", fps.GetActualFps());
            DxLib::DrawString(x, y, buffer, color);
            y += 18;
        }
    }
}