//==============================================================================
// 
//  Project: mm2hack
//  FixedStar.h
// 
//  A class that draws stars fixed in outer space.
// 
//==============================================================================
#pragma once

#include "StarState.h"

namespace mm2hack::apps::vfx::stareffects
{
    // Fixed star effect class
    class FixedStar
    {
    public:
        FixedStar(int tileIndex, float x, float y);
        FixedStar(const FixedStarState& s);
        ~FixedStar() = default;

        void Draw() const;
        FixedStarState ToState() const;
    private:
        int _tileIndex;     // Tile index for the star sprite
        float _x, _y;       // Position
    };
}