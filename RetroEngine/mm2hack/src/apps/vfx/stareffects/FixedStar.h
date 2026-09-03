//==============================================================================
// 
//  Project: mm2hack
//  FixedStar.h
// 
//  A class that draws stars fixed in outer space.
// 
//==============================================================================
#pragma once

#include "apps/rendering/sprite/SpriteManager.h"
#include "StarState.h"

#include <string>

namespace mm2hack::apps::vfx::stareffects
{
    // Fixed star effect class
    class FixedStar
    {
    public:
        FixedStar(int tileIndex, float x, float y);
        FixedStar(const FixedStarState& s);
        ~FixedStar() = default;

        void Draw(const rendering::sprite::SpriteManager& sprites,
                  rendering::sprite::SpriteManager::Id sprite_id) const;
        FixedStarState ToState() const;

    private:
        const std::wstring kClassName{ L"FixedStar" };

        int _tileIndex;     // Tile index for the star sprite
        float _x, _y;       // Position
    };
}
