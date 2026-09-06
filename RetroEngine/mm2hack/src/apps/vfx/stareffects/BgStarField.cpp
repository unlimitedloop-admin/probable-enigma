#include "pch.h"

#include "BgStarField.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <istream>
#include <ostream>
#include <random>
#include <string>
#include <vector>
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
        constexpr std::uint32_t kStateVersion = 2;
        constexpr std::uint32_t kMaximumFixedStars = 256;
        constexpr std::uint32_t kMaximumMovingStars = 1024;
        constexpr std::array<int, 8> kPaletteColumns{ 1, 2, 3, 5, 6, 8, 9, 10 };

        struct StarFieldState final
        {
            std::uint32_t pattern_id{};
            std::uint64_t elapsed_ticks{};
            std::vector<FixedStarState> fixed_stars{};
            std::vector<StarState> stars{};
        };

        std::uint32_t Mix32(std::uint32_t value) noexcept
        {
            value ^= value >> 16;
            value *= 0x7FEB352Du;
            value ^= value >> 15;
            value *= 0x846CA68Bu;
            value ^= value >> 16;
            return value;
        }

        std::uint32_t Sample(
            std::uint32_t pattern_id, std::uint64_t tick, std::uint32_t stream) noexcept
        {
            const auto low = static_cast<std::uint32_t>(tick);
            const auto high = static_cast<std::uint32_t>(tick >> 32);
            return Mix32(pattern_id ^ Mix32(low) ^ Mix32(high + 0x9E3779B9u) ^
                Mix32(stream + 0x85EBCA6Bu));
        }

        std::uint32_t CreatePatternId()
        {
            // Entropy is consumed only when a new visual pattern is created.
            // The resulting ID is persisted; simulation never queries this source.
            std::random_device entropy;
            const auto first = static_cast<std::uint32_t>(entropy());
            const auto second = static_cast<std::uint32_t>(entropy());
            const auto result = Mix32(first ^ Mix32(second));
            return result != 0 ? result : 0x5EED1234u;
        }

        rendering::sprite::SpriteManager::Id LoadStarSprite(std::uint32_t pattern_id)
        {
            using SpriteManager = rendering::sprite::SpriteManager;
            auto& sprites = runtime::GameContext::GetInstance().GetResourceManager().GetSpriteManager();
            const auto scheme = Sample(pattern_id, 0, 0) % kPaletteColumns.size();
            const std::wstring resource_name =
                std::wstring(kStarSpriteName) + L"/SCHEME_" + std::to_wstring(scheme);

            bool created = false;
            const auto sprite_id = sprites.Load(
                resource_name, MM2H_GRAPHICS(FlashStar), MM2H_GRAPHPROPS(FlashStar), &created);
            if (sprite_id == static_cast<SpriteManager::Id>(-1) || !created)
            {
                return sprite_id;
            }

            const int palette_column = kPaletteColumns[scheme];
            std::array<rendering::sprite::SpriteAtlas::PaletteColorMapping, 4> mappings{};
            for (int row = 0; row < 4; ++row)
            {
                mappings[static_cast<std::size_t>(row)] = {
                    .source_palette_index = row * 16,
                    .target_palette_index = row * 16 + palette_column,
                };
            }
            if (!sprites.ReplacePaletteColorsById(sprite_id, mappings))
            {
                sprites.ReleaseById(sprite_id);
                return static_cast<SpriteManager::Id>(-1);
            }
            return sprite_id;
        }

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

        bool ParseState(std::istream& in, StarFieldState& state)
        {
            core::save::StateReader reader(in);
            std::uint32_t state_version{};
            if (!reader.ReadU32(state_version) || state_version != kStateVersion ||
                !reader.ReadU32(state.pattern_id) ||
                !reader.ReadU64(state.elapsed_ticks))
            {
                return false;
            }

            std::uint32_t fixed_count{};
            if (!reader.ReadU32(fixed_count) || fixed_count > kMaximumFixedStars)
            {
                return false;
            }
            state.fixed_stars.clear();
            state.fixed_stars.reserve(fixed_count);
            for (std::uint32_t i = 0; i < fixed_count; ++i)
            {
                FixedStarState fixed_star{};
                if (!fixed_star.Load(reader) || !IsValid(fixed_star)) return false;
                state.fixed_stars.push_back(fixed_star);
            }

            std::uint32_t count{};
            if (!reader.ReadU32(count) || count > kMaximumMovingStars)
            {
                return false;
            }
            state.stars.clear();
            state.stars.reserve(count);
            for (std::uint32_t i = 0; i < count; ++i)
            {
                StarState star{};
                if (!star.Load(reader) || !IsValid(star)) return false;
                state.stars.push_back(star);
            }
            return reader.Good();
        }
    }

    void BgStarField::InitStars()
    {
        InitStars(CreatePatternId());
    }

    void BgStarField::InitStars(std::uint32_t pattern_id)
    {
        _stars.clear();
        _fixedStars.clear();
        _pattern_id = pattern_id;
        _elapsed_ticks = 0;

        _sprite_id = LoadStarSprite(_pattern_id);
        if (_sprite_id == static_cast<rendering::sprite::SpriteManager::Id>(-1))
        {
            THROW_EXCEPTION(L"The image data is invalid: " + std::wstring(kStarSpriteName), kClassName);
        }

        // Fixed stars setup.
        for (int i = 0; i < 50; ++i)
        {
            const auto index = static_cast<std::uint64_t>(i);
            const int tileIndex = 4 + static_cast<int>(Sample(_pattern_id, index, 1) % 3);
            const float x = static_cast<float>(
                Sample(_pattern_id, index, 2) % (config::SystemConfig::kScreenWidth + 1));
            const float y = static_cast<float>(
                Sample(_pattern_id, index, 3) % (config::SystemConfig::kScreenHeight + 1));
            _fixedStars.emplace_back(std::make_unique<FixedStar>(tileIndex, x, y));
        }
    }

    void BgStarField::UpdateStars()
    {
        ++_elapsed_ticks;
        if ((Sample(_pattern_id, _elapsed_ticks, 10) & 7U) == 0U)
        {
            const int type = static_cast<int>(Sample(_pattern_id, _elapsed_ticks, 11) % 3);
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
            if ((Sample(_pattern_id, _elapsed_ticks, 12) & 1U) == 0U)
            {
                startX = static_cast<float>(
                    Sample(_pattern_id, _elapsed_ticks, 13) %
                    (config::SystemConfig::kScreenWidth + 1));
                startY = 0.0f;
            }
            else
            {
                startX = static_cast<float>(config::SystemConfig::kScreenWidth);
                startY = static_cast<float>(
                    Sample(_pattern_id, _elapsed_ticks, 14) %
                    (config::SystemConfig::kScreenHeight + 1));
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
        if (!writer.WriteU32(kStateVersion) || !writer.WriteU32(_pattern_id) ||
            !writer.WriteU64(_elapsed_ticks))
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

    bool BgStarField::Validate(std::istream& in)
    {
        StarFieldState state{};
        return ParseState(in, state);
    }

    bool BgStarField::Load(std::istream& in)
    {
        StarFieldState state{};
        if (!ParseState(in, state)) return false;

        const auto sprite_id = LoadStarSprite(state.pattern_id);
        if (sprite_id == static_cast<rendering::sprite::SpriteManager::Id>(-1))
        {
            return false;
        }

        std::vector<std::unique_ptr<FixedStar>> fixed_stars;
        fixed_stars.reserve(state.fixed_stars.size());
        for (const auto& fixed_star : state.fixed_stars)
        {
            fixed_stars.emplace_back(std::make_unique<FixedStar>(fixed_star));
        }
        std::vector<std::unique_ptr<Star>> stars;
        stars.reserve(state.stars.size());
        for (const auto& star : state.stars)
        {
            stars.emplace_back(std::make_unique<Star>(star));
        }

        _pattern_id = state.pattern_id;
        _sprite_id = sprite_id;
        _elapsed_ticks = state.elapsed_ticks;
        _fixedStars.swap(fixed_stars);
        _stars.swap(stars);
        return true;
    }
}
