//==============================================================================
//
//  Project: mm2hack
//  PlayerSession.h
//
//  Player-owned progress that outlives a single attempt at a stage: today just
//  the number of lives. A miss rebuilds the whole phase (and every entity in
//  it), so none of this can live on PlayerEntity or the phase -- the scene
//  owns one PlayerSession and hands phases a reference through IPhaseHost.
//
//==============================================================================
#pragma once

#include <algorithm>
#include <cstdint>

#include "core/save/StateIO.h"

namespace mm2hack::apps::world::session
{
    struct PlayerSessionState final
    {
        std::int32_t lives{ 3 };

        [[nodiscard]] bool IsValid() const noexcept;
        bool Save(core::save::StateWriter& writer) const;
        bool Load(core::save::StateReader& reader);
    };

    class PlayerSession final
    {
    public:
        // `lives` counts the life in play too, like the original: the HUD shows
        // lives - 1 ("02" on a fresh start), and a miss at "00" (lives == 1)
        // is the one that ends the game.
        static constexpr int kInitialLives{ 3 };
        static constexpr int kMaxDisplayLives{ 99 };
        static constexpr int kMaxLives{ kMaxDisplayLives + 1 };

        [[nodiscard]] int Lives() const noexcept { return _lives; }
        // Spare lives as the HUD shows them (2 digits, "00" - "99").
        [[nodiscard]] int DisplayLives() const noexcept { return std::max(_lives - 1, 0); }
        [[nodiscard]] bool IsOutOfLives() const noexcept { return _lives <= 0; }

        // 1UP items, score thresholds, cheats... Clamped to kMaxLives; negative
        // counts are ignored (use LoseLife()).
        void AddLives(int count = 1) noexcept
        {
            if (count > 0)
            {
                _lives = std::min(_lives + count, kMaxLives);
            }
        }

        // Called once per miss. Returns false when that miss used the last life
        // (GAME OVER -- not implemented yet, the caller decides what to do).
        bool LoseLife() noexcept
        {
            _lives = std::max(_lives - 1, 0);
            return _lives > 0;
        }

        void Reset() noexcept { _lives = kInitialLives; }

        [[nodiscard]] PlayerSessionState CaptureState() const noexcept { return { _lives }; }
        bool RestoreState(const PlayerSessionState& state) noexcept
        {
            if (!state.IsValid())
            {
                return false;
            }
            _lives = state.lives;
            return true;
        }

    private:
        int _lives{ kInitialLives };
    };

    inline bool PlayerSessionState::IsValid() const noexcept
    {
        return lives >= 0 && lives <= PlayerSession::kMaxLives;
    }

    inline bool PlayerSessionState::Save(core::save::StateWriter& writer) const
    {
        return IsValid() && writer.WriteI32(lives);
    }

    inline bool PlayerSessionState::Load(core::save::StateReader& reader)
    {
        PlayerSessionState loaded{};
        if (!reader.ReadI32(loaded.lives) || !loaded.IsValid())
        {
            return false;
        }
        *this = loaded;
        return true;
    }
}
