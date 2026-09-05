//==============================================================================
// 
//  Project: mm2hack
//  BgStarField.h
// 
//  Background star field effect vfx.
// 
//==============================================================================
#pragma once

#include <cstdint>
#include <iostream>
#include <istream>
#include <memory>
#include <string>
#include <vector>
#include "apps/rendering/sprite/SpriteManager.h"
#include "FixedStar.h"
#include "Star.h"

namespace mm2hack::apps::vfx::stareffects
{
    // Star field animation class
    class BgStarField
    {
    public:
        BgStarField() = default;
        ~BgStarField() = default;

        // Load the star field background
        void InitStars();
        // Update the stars every frame
        void UpdateStars();
        // Draw the stars on the screen
        void DrawStars();

        // === Save/Load state ===
        bool Save(std::ostream& out) const;
        bool Load(std::istream& in);

    private:
        const std::wstring kClassName{ L"BgStarField" };

        rendering::sprite::SpriteManager::Id _sprite_id{
            static_cast<rendering::sprite::SpriteManager::Id>(-1)
        };
        std::vector<std::unique_ptr<Star>> _stars;              // Moving stars (shooting stars)
        std::vector<std::unique_ptr<FixedStar>> _fixedStars;    // Fixed stars
        std::uint64_t _elapsed_ticks{};                         // Deterministic spawn schedule tick
    };
}
