//==============================================================================
//
//  Project: mm2hack
//  HealthMeterUI.h
//
//  The player's vitality meter: a fighting-game style horizontal bar (no
//  partitions) with an emblem on its left carrying the weapon initials and
//  the lives count. The static frame is a sprite (HEALTH_METER_N0_ALL_PATTERN);
//  the fill is drawn on top of it in NES palette colors, so its length,
//  color tier, chip-damage trail and low-HP blink can change every tick.
//  Look and timing come from assets/data/ui/HEALTH_METER.json.
//
//==============================================================================
#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "apps/rendering/sprite/SpriteManager.h"

namespace mm2hack::apps::ui::hud
{
    // What a bar row is painted with while filled (see HealthMeterStyle::row_roles).
    enum class HealthMeterRowRole : std::uint8_t
    {
        Light,
        Highlight,
        Main,
        Dark
    };

    // One color tier of the fill, picked by the current HP ratio.
    struct HealthMeterTone final
    {
        int above_percent{ 0 };     // Used while HP% is strictly above this (tiers are checked in order)
        int light{ 0x3A };          // NES palette indices
        int main{ 0x2A };
        int dark{ 0x1A };
    };

    // Everything configurable about the meter. Defaults match the approved
    // mockup "C" and the HEALTH_METER json shipped alongside it.
    struct HealthMeterStyle final
    {
        int x{ 8 };                         // Screen position of the frame sprite's top-left
        int y{ 8 };

        // Fill area, relative to the frame: `bar_rows` rows starting at
        // (bar_x, bar_y); row r spans bar_length_px plus
        // (bar_rows - 1 - r) * bar_slant_px extra pixels, giving the slanted
        // right end (and the slanted fill front) of the frame art.
        int bar_x{ 28 };
        int bar_y{ 8 };
        int bar_rows{ 8 };
        int bar_length_px{ 112 };
        int bar_slant_px{ 1 };
        std::vector<HealthMeterRowRole> row_roles{
            HealthMeterRowRole::Light, HealthMeterRowRole::Highlight,
            HealthMeterRowRole::Main, HealthMeterRowRole::Main, HealthMeterRowRole::Main,
            HealthMeterRowRole::Dark, HealthMeterRowRole::Dark, HealthMeterRowRole::Dark,
        };

        int initials_x{ 4 };                // Weapon initials (8x8 font), relative to the frame
        int initials_y{ 3 };
        int lives_x{ 4 };                   // Lives count, two digits ("03")
        int lives_y{ 12 };

        std::vector<HealthMeterTone> tones{
            HealthMeterTone{ 50, 0x3A, 0x2A, 0x1A },
            HealthMeterTone{ 25, 0x38, 0x28, 0x18 },
            HealthMeterTone{ 0,  0x26, 0x16, 0x06 },
        };
        int highlight{ 0x30 };              // Row role Highlight, in every tier

        int chip_flash{ 0x30 };             // Lost HP, for the first chip_flash_frames after a hit
        int chip{ 0x26 };                   // Lost HP, after that, until drained
        int chip_hold_frames{ 24 };         // Lost HP lingers this long before draining
        int chip_flash_frames{ 12 };
        int chip_drain_interval{ 2 };       // Ticks per HP drained from the trail

        int refill_interval{ 3 };           // Ticks per HP while the fill catches up to a heal
        int low_hp_threshold{ 0x04 };       // At or below this HP (but alive) the fill blinks
        int low_hp_flash{ 0x30 };
        int low_hp_blink_frames{ 8 };       // Half-period of that blink
    };

    class HealthMeterUI final
    {
    public:
        // Reads a style JSON. On any failure returns false and leaves `out`
        // untouched, so a caller can simply keep the built-in defaults.
        static bool LoadStyle(const std::wstring& filepath, HealthMeterStyle& out);

        void SetStyle(const HealthMeterStyle& style) { _style = style; }
        // Snaps every animated value straight to `hp` (stage start, save-state load).
        void Reset(int hp, int max_hp) noexcept;
        // Latest HP to animate toward. A drop starts the chip-damage trail.
        void SetTarget(int hp, int max_hp) noexcept;
        // One tick of the refill / chip-drain / blink animation.
        void Tick() noexcept;
        void Render(rendering::sprite::SpriteManager::Id frame_sprite, std::wstring_view initials, int lives) const;

    private:
        [[nodiscard]] const HealthMeterTone& currentTone_() const noexcept;
        [[nodiscard]] int toFillPx_(int hp) const noexcept;

    private:
        HealthMeterStyle _style{};
        int _max_hp{ 1 };
        int _target_hp{ 0 };    // Where the fill is heading
        int _shown_hp{ 0 };     // What the fill shows right now
        int _chip_hp{ 0 };      // Where the chip-damage trail ends (>= _shown_hp)
        int _chip_hold{ 0 };    // Ticks left before the trail starts draining
        int _frame{ 0 };        // Free-running tick counter (refill/drain cadence, blink)
    };
}
