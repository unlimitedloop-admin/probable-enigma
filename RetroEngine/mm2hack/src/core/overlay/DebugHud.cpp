#include "pch.h"

#include "DebugHud.h"

#include <cstdio>
#include "config/ConfigUIManager.h"
#include "utils/FpsManager.h"

namespace mm2hack::core::overlay
{
    void DebugHud::Draw() const
    {
        if (!config::ConfigUIManager::GetCurrentHudConfig().showFps)
            return;

        auto& fps = utils::FpsManager::GetInstance();
        wchar_t buffer[64];
        swprintf(buffer, 64, L"FPS: %.1f", fps.GetActualFps());
        DxLib::DrawString(10, 10, buffer, DxLib::GetColor(255, 255, 255));
    }
}