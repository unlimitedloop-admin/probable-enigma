//==============================================================================
// 
//  Project: mm2hack
//  TwinkleCursorAnimator.h
// 
//  Creates a cursor that twinkles by cycling through sprite frames.
// 
//==============================================================================
#pragma once

#include <array>
#include <cstdint>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>
#include "apps/graphics/sprite/SpriteManager.h"

namespace mm2hack::apps::vfx::cursor
{
    // Display a twinkling cursor animation at the specified position
    class TwinkleCursorAnimator final
    {
    public:
        using SpriteManager = graphics::sprite::SpriteManager;

        struct Step
        {
            int tile;      // Tile number to use (frame)
            int duration;  // Number of frames to maintain this tile (1=1 game frame)
        };

        // Fade animator default loop (7 steps)
        static constexpr std::array<Step, 7> kDefaultFadeLoop{
            Step{0, 12}, Step{1, 2}, Step{2, 2}, Step{3, 2}, Step{2, 2}, Step{1, 2}, Step{0, 6}
        };

    public:
        TwinkleCursorAnimator(SpriteManager& sprites, int x = 0, int y = 0) noexcept;
        ~TwinkleCursorAnimator() = default;

        // Load sprite resources for the cursor animation
        bool Load(const std::wstring& name, std::wstring_view png_path, std::wstring_view json_path);
        // Set the position of the cursor
        void SetPosition(int x, int y) noexcept;
        // Get the current position of the cursor
        [[nodiscard]] int X() const noexcept { return _x; }
        [[nodiscard]] int Y() const noexcept { return _y; }
        // Reset the animation to the beginning
        void Reset() noexcept;

        // Update the animation state (to be called every frame)
        void Update() noexcept;
        // Draw the cursor at its current position
        void Draw() const noexcept;
        void DrawAt(int x, int y) const noexcept;

        // Set a custom animation loop
        void SetLoop(const Step* steps, std::size_t count) noexcept;
        // Set the base duration for each tile in the default fade loop
        void SetBaseTileDuration(int frames) noexcept;
        // Get the current tile index being displayed
        [[nodiscard]] int CurrentTile() const noexcept { return _steps[_stepIndex].tile; }

    private:
        void drawImpl_(int x, int y) const noexcept;     // Internal draw implementation

    private:
        SpriteManager& _sprites;
        std::wstring   _name;
        int            _x{ 0 };
        int            _y{ 0 };

        using Id = std::int32_t;
        Id _id{ -1 };

        std::vector<Step> _steps{ std::begin(kDefaultFadeLoop), std::end(kDefaultFadeLoop) };   // Animation steps
        std::size_t       _stepIndex{ 0 };  // Current step index
        int               _ticks{ 0 };      // Ticks in the current step
    };
}