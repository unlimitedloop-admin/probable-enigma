#include "pch.h"

#include "BgStarField.h"

#include <cstdlib>
#include <ctime>
#include <istream>
#include <ostream>
#include "apps/deal/GameContext.h"
#include "config/GameAssets.h"
#include "FixedStar.h"
#include "Star.h"
#include "StarState.h"

namespace mm2hack::apps::vfx::stareffects
{
    void BgStarField::InitStars()
    {
        srand(static_cast<unsigned int>(time(nullptr)));
        auto& sprites = deal::GameContext::GetInstance().GetResourceManager().GetSpriteManager();
        sprites.Load(std::wstring(kStarSpriteName), MM2H_GRAPHICS(FlashStar), MM2H_PROPERTIES(FlashStar));

        // Change the color for the stars.
        auto throwImageDataException = [&](const wchar_t* msg) {
            THROW_EXCEPTION(L"The image data is invalid: " + std::wstring(kStarSpriteName), kClassName);
        };
        if (!sprites.ReplacePaletteColorByName(std::wstring(kStarSpriteName), 1, 0)) throwImageDataException(L"palette 0->1");
        if (!sprites.ReplacePaletteColorByName(std::wstring(kStarSpriteName), 17, 16)) throwImageDataException(L"palette 16->17");
        if (!sprites.ReplacePaletteColorByName(std::wstring(kStarSpriteName), 33, 32)) throwImageDataException(L"palette 32->33");
        if (!sprites.ReplacePaletteColorByName(std::wstring(kStarSpriteName), 49, 48)) throwImageDataException(L"palette 48->49");
        if (!sprites.ApplyRandomColorFilterByName(std::wstring(kStarSpriteName))) throwImageDataException(L"random color filter");

        // Fixed stars setup.
        for (int i = 0; i < 50; ++i)
        {
            int tileIndex = 4 + rand() % 3; // 4: Flash, 5: Bright, 6: Dim
            float x = static_cast<float>(rand() % 257); // 0-256 axis inclusive
            float y = static_cast<float>(rand() % 241); // 0-240 axis inclusive
            _fixedStars.emplace_back(std::make_unique<FixedStar>(tileIndex, x, y));
        }
    }

    void BgStarField::UpdateStars()
    {
        // Add new stars randomly
        if (rand() % 8 == 0)
        {
            int type = rand() % 3;
            float vx, vy;
            switch (type)
            {
            case 0: // Flashing star
                vx = -2.5f;
                vy = 2.0f;
                break;
            case 1: // Twinkling star
                vx = -0.8f;
                vy = 0.5f;
                break;
            case 2: // Darker star
                vx = -0.2f;
                vy = 0.2f;
                break;
            default:
                vx = -0.1f;
                vy = 0.1f;
                break;
            }

            float startX, startY;
            if (rand() % 2 == 0)
            {
                startX = static_cast<float>(rand() % (config::SystemConfig::kScreenWidth + 1));     // 0-256
                startY = 0.0f;
            }
            else
            {
                startX = static_cast<float>(config::SystemConfig::kScreenWidth);
                startY = static_cast<float>(rand() % (config::SystemConfig::kScreenHeight + 1));    // 0-240
            }

            _stars.emplace_back(std::make_unique<Star>(type, startX, startY, vx, vy));
        }

        // Update the existing stars and remove those that are off-screen.
        for (auto it = _stars.begin(); it != _stars.end(); )
        {
            (*it)->Update();
            if ((*it)->IsOffScreen())
            {
                it = _stars.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void BgStarField::DrawStars()
    {
        for (auto& s : _stars)
        {
            s->Draw();
        }

        for (auto& s : _fixedStars)
        {
            s->Draw();
        }
    }

    void BgStarField::Save(std::ostream& out) const
    {
        // Fixed stars
        size_t fixedCount = _fixedStars.size();
        out.write(reinterpret_cast<const char*>(&fixedCount), sizeof(fixedCount));
        for (const auto& star : _fixedStars)
        {
            FixedStarState state = star->ToState();
            state.Save(out);
        }

        // Shooting stars
        size_t count = _stars.size();
        out.write(reinterpret_cast<const char*>(&count), sizeof(count));
        for (const auto& star : _stars)
        {
            StarState state = star->ToState();
            state.Save(out);
        }
    }

    void BgStarField::Load(std::istream& in)
    {
        _fixedStars.clear();
        _stars.clear();

        size_t fixedCount = 0;
        in.read(reinterpret_cast<char*>(&fixedCount), sizeof(fixedCount));
        for (size_t i = 0; i < fixedCount; ++i)
        {
            FixedStarState state{};
            state.Load(in);
            _fixedStars.emplace_back(std::make_unique<FixedStar>(state));
        }

        size_t count = 0;
        in.read(reinterpret_cast<char*>(&count), sizeof(count));
        for (size_t i = 0; i < count; ++i)
        {
            StarState state{};
            state.Load(in);
            _stars.emplace_back(std::make_unique<Star>(state));
        }
    }
}