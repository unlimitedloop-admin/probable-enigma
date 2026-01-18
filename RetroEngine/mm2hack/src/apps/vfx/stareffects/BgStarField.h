//==============================================================================
// 
//  Project: mm2hack
//  BgStarField.h
// 
//  Background star field effect vfx.
// 
//==============================================================================
#pragma once

#include <iostream>
#include <istream>
#include <memory>
#include <string>
#include <vector>
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
        void Save(std::ostream& out) const;
        void Load(std::istream& in);

    private:
        const std::wstring kClassName{ L"BgStarField" };

        std::vector<std::unique_ptr<Star>> _stars;              // Moving stars (shooting stars)
        std::vector<std::unique_ptr<FixedStar>> _fixedStars;    // Fixed stars
    };
}