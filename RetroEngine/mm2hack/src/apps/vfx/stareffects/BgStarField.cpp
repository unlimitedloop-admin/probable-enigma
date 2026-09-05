#include "pch.h"

#include "BgStarField.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <istream>
#include <ostream>
#include "apps/runtime/GameContext.h"
#include "config/GameAssets.h"
#include "core/save/StateIO.h"
#include "FixedStar.h"
#include "Star.h"
#include "StarState.h"

namespace mm2hack::apps::vfx::stareffects
{
    namespace
    {
        constexpr std::wstring_view kStarSpriteName = L"STARS";
        constexpr std::uint32_t kStateVersion = 1;
        constexpr std::uint32_t kMaximumFixedStars = 256;
        constexpr std::uint32_t kMaximumMovingStars = 1024;
        constexpr std::uint64_t kSpawnCycleTicks = 96;

        struct StarSpawnStep final
        {
            std::uint8_t tick;
            std::int32_t type;
            bool from_top;
            float coordinate;
        };

        constexpr std::array kSpawnPattern{
            StarSpawnStep{  4, 0, true,   226.0f },
            StarSpawnStep{ 10, 2, false,   42.0f },
            StarSpawnStep{ 19, 1, true,    71.0f },
            StarSpawnStep{ 25, 2, true,   154.0f },
            StarSpawnStep{ 38, 0, false,   18.0f },
            StarSpawnStep{ 44, 1, false,  137.0f },
            StarSpawnStep{ 55, 2, true,    23.0f },
            StarSpawnStep{ 63, 1, true,   198.0f },
            StarSpawnStep{ 70, 0, false,  211.0f },
            StarSpawnStep{ 78, 2, false,   83.0f },
            StarSpawnStep{ 87, 1, true,   117.0f },
            StarSpawnStep{ 94, 2, false,  169.0f },
        };

        bool IsFiniteAndReasonable(float value) noexcept
        {
            return std::isfinite(value) && value >= -4096.0f && value <= 4096.0f;
        }

        bool IsValid(const FixedStarState& state) noexcept
        {
            return state.tileIndex >= 4 && state.tileIndex <= 6 &&
                IsFiniteAndReasonable(state.x) && IsFiniteAndReasonable(state.y);
        }

        bool IsValid(const StarState& state) noexcept
        {
            return state.type >= 0 && state.type <= 2 &&
                IsFiniteAndReasonable(state.x) && IsFiniteAndReasonable(state.y) &&
                IsFiniteAndReasonable(state.vx) && IsFiniteAndReasonable(state.vy);
        }
    }

    void BgStarField::InitStars()
    {
        _stars.clear();
        _fixedStars.clear();
        _elapsed_ticks = 0;

        auto& sprites = runtime::GameContext::GetInstance().GetResourceManager().GetSpriteManager();
        _sprite_id = sprites.Load(std::wstring(kStarSpriteName), MM2H_GRAPHICS(FlashStar), MM2H_GRAPHPROPS(FlashStar));

        // Change the color for the stars.
        auto throwImageDataException = [&](const wchar_t* msg) {
            THROW_EXCEPTION(L"The image data is invalid: " + std::wstring(kStarSpriteName), kClassName);
        };
        if (!sprites.ReplacePaletteColorById(_sprite_id, 1, 0)) throwImageDataException(L"palette 0->1");
        if (!sprites.ReplacePaletteColorById(_sprite_id, 17, 16)) throwImageDataException(L"palette 16->17");
        if (!sprites.ReplacePaletteColorById(_sprite_id, 33, 32)) throwImageDataException(L"palette 32->33");
        if (!sprites.ReplacePaletteColorById(_sprite_id, 49, 48)) throwImageDataException(L"palette 48->49");
        constexpr int kStarHueShift = 48;
        if (!sprites.ApplyHueFilterById(_sprite_id, kStarHueShift)) throwImageDataException(L"hue filter");

        // Fixed stars setup.
        for (int i = 0; i < 50; ++i)
        {
            const int tileIndex = 4 + (i % 3); // 4: Flash, 5: Bright, 6: Dim
            const float x = static_cast<float>((i * 73 + 19) % 257);
            const float y = static_cast<float>((i * 151 + 37) % 241);
            _fixedStars.emplace_back(std::make_unique<FixedStar>(tileIndex, x, y));
        }
    }

    void BgStarField::UpdateStars()
    {
        ++_elapsed_ticks;
        const auto cycle_tick = static_cast<std::uint8_t>(_elapsed_ticks % kSpawnCycleTicks);
        const auto spawn_it = std::find_if(
            kSpawnPattern.begin(), kSpawnPattern.end(),
            [cycle_tick](const StarSpawnStep& step) { return step.tick == cycle_tick; });

        if (spawn_it != kSpawnPattern.end())
        {
            const int type = spawn_it->type;
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

            float startX{};
            float startY{};
            if (spawn_it->from_top)
            {
                startX = spawn_it->coordinate;
                startY = 0.0f;
            }
            else
            {
                startX = static_cast<float>(config::SystemConfig::kScreenWidth);
                startY = spawn_it->coordinate;
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
        const auto& sprites = runtime::GameContext::GetInstance().GetResourceManager().GetSpriteManager();
        for (auto& s : _stars)
        {
            s->Draw(sprites, _sprite_id);
        }

        for (auto& s : _fixedStars)
        {
            s->Draw(sprites, _sprite_id);
        }
    }

    bool BgStarField::Save(std::ostream& out) const
    {
        core::save::StateWriter writer(out);
        if (!writer.WriteU32(kStateVersion) || !writer.WriteU64(_elapsed_ticks))
        {
            return false;
        }

        // Fixed stars
        if (_fixedStars.size() > kMaximumFixedStars ||
            !writer.WriteU32(static_cast<std::uint32_t>(_fixedStars.size())))
        {
            return false;
        }
        for (const auto& star : _fixedStars)
        {
            FixedStarState state = star->ToState();
            if (!IsValid(state) || !state.Save(writer)) return false;
        }

        // Shooting stars
        if (_stars.size() > kMaximumMovingStars ||
            !writer.WriteU32(static_cast<std::uint32_t>(_stars.size())))
        {
            return false;
        }
        for (const auto& star : _stars)
        {
            StarState state = star->ToState();
            if (!IsValid(state) || !state.Save(writer)) return false;
        }
        return writer.Good();
    }

    bool BgStarField::Load(std::istream& in)
    {
        core::save::StateReader reader(in);
        std::uint32_t state_version{};
        std::uint64_t elapsed_ticks{};
        if (!reader.ReadU32(state_version) || state_version != kStateVersion ||
            !reader.ReadU64(elapsed_ticks))
        {
            return false;
        }

        std::uint32_t fixed_count{};
        if (!reader.ReadU32(fixed_count) || fixed_count > kMaximumFixedStars)
        {
            return false;
        }
        std::vector<std::unique_ptr<FixedStar>> fixed_stars;
        fixed_stars.reserve(fixed_count);
        for (std::uint32_t i = 0; i < fixed_count; ++i)
        {
            FixedStarState state{};
            if (!state.Load(reader) || !IsValid(state)) return false;
            fixed_stars.emplace_back(std::make_unique<FixedStar>(state));
        }

        std::uint32_t count{};
        if (!reader.ReadU32(count) || count > kMaximumMovingStars)
        {
            return false;
        }
        std::vector<std::unique_ptr<Star>> stars;
        stars.reserve(count);
        for (std::uint32_t i = 0; i < count; ++i)
        {
            StarState state{};
            if (!state.Load(reader) || !IsValid(state)) return false;
            stars.emplace_back(std::make_unique<Star>(state));
        }

        _elapsed_ticks = elapsed_ticks;
        _fixedStars.swap(fixed_stars);
        _stars.swap(stars);
        return true;
    }
}
