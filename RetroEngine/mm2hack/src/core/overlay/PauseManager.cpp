#include "pch.h"

#include "PauseManager.h"

namespace mm2hack::core::overlay
{
    void PauseManager::SetPaused(bool paused)
    {
        _isPaused = paused;
    }

    bool PauseManager::IsPaused()
    {
        return _isPaused;
    }

    void PauseManager::Toggle()
    {
        _isPaused = !_isPaused;
    }

    void PauseManager::DrawOverlay()
    {
        using conf = config::SystemConfig;
        int w = conf::kScreenWidth;
        int h = conf::kScreenHeight;

        ::DxLib::SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
        ::DxLib::DrawBox(0, 0, w, h, ::DxLib::GetColor(0, 0, 0), TRUE);
        ::DxLib::SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        ::DxLib::DrawString(w / 2 - 40, h / 2 - 8, L"PAUSED", ::DxLib::GetColor(255, 255, 255));
    }
}