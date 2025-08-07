#include "pch.h"

#include "DebugHud.h"

#include <cstdio>
#include "utils/FpsManager.h"   // FpsManager::GetInstance()

namespace mm2hack::core::overlay
{
    void DebugHud::Draw() const
    {
        auto& fps = utils::FpsManager::GetInstance();
        wchar_t buffer[64];
        swprintf(buffer, 64, L"FPS: %.1f", fps.GetActualFps());
        DxLib::DrawString(10, 10, buffer, GetColor(255, 255, 255));
    }
}