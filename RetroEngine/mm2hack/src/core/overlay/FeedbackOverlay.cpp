#include "pch.h"

#include "FeedbackOverlay.h"

#include "core/winapi/WindowManager.h"

namespace mm2hack::core::overlay
{
    constexpr int GAME_W = 256;
    constexpr int GAME_H = 240;
    constexpr int SCALE = 4;
    constexpr int VIRTUAL_W = GAME_W * SCALE;
    constexpr int VIRTUAL_H = GAME_H * SCALE;

    FeedbackOverlay::FeedbackOverlay()
    {
        _fontScreen = DxLib::MakeScreen(VIRTUAL_W, VIRTUAL_H, TRUE);
        _fontHandle = DxLib::CreateFontToHandle(L"Segoe UI", 64, 3, DX_FONTTYPE_ANTIALIASING_EDGE);
    }

    FeedbackOverlay::~FeedbackOverlay()
    {
        if (_fontScreen != -1)
        {
            DeleteGraph(_fontScreen);
        }
        if (_fontHandle != -1)
        {
            DxLib::DeleteFontToHandle(_fontHandle);
        }
    }

    void FeedbackOverlay::ShowMessage(const std::wstring& message, int duration)
    {
        _messages.push_back({ message, 0, duration });
    }

    void FeedbackOverlay::Update()
    {
        if (_messages.empty()) return;

        _messages.front().frame++;
        if (_messages.front().frame >= _messages.front().duration)
        {
            _messages.pop_front();
        }
    }

    void FeedbackOverlay::Render()
    {
        if (_messages.empty() || _fontScreen == -1) return;

        const Message& msg = _messages.front();

        const int animIn = 30;
        const int animOut = 30;
        const int stayFrames = std::max(0, msg.duration - animIn - animOut);

        float xFactor = 1.0f;       // For display position adjustment
        float alphaFactor = 1.0f;   // Alpha value (transparency)

        if (msg.frame < animIn)
        {
            // Ease-In.
            float t = msg.frame / static_cast<float>(animIn);
            xFactor = 1.0f - (1.0f - t) * (1.0f - t);   // Ease-In curve
        }
        else if (msg.frame >= msg.duration - animOut)
        {
            // Ease-Out.
            float t = (msg.duration - msg.frame) / static_cast<float>(animOut);
            xFactor = t * t;
            alphaFactor = t;
        }
        else
        {
            // Center stationary period.
            xFactor = 1.0f;
            alphaFactor = 1.0f;
        }

        const int baseX = -160;
        const int targetX = 16;
        const int drawY_base = 210;
        int drawX = static_cast<int>((baseX + (targetX - baseX) * xFactor) * SCALE);
        int drawY = drawY_base * SCALE;
        int alpha = static_cast<int>(255 * alphaFactor);

        // Draw on a virtual screen.
        DxLib::SetDrawScreen(_fontScreen);
        DxLib::ClearDrawScreen();

        //DxLib::SetFontSize(60);
        DxLib::SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha / 2);
        DxLib::DrawBox(drawX - 16, drawY - 12, drawX + 640, drawY + 60, GetColor(0, 0, 0), TRUE);

        DxLib::SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
        DxLib::DrawStringToHandle(drawX, drawY, msg.text.c_str(), GetColor(255, 255, 255), _fontHandle);
        DxLib::SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        // Draws into the window you are showing.
        DxLib::SetDrawScreen(core::winapi::WindowManager::GetInstance().GetScreenHandle());
        DxLib::SetDrawMode(DX_DRAWMODE_BILINEAR);
        DxLib::DrawExtendGraph(0, 0, GAME_W, GAME_H, _fontScreen, TRUE);
        DxLib::SetDrawMode(DX_DRAWMODE_NEAREST);
    }
}