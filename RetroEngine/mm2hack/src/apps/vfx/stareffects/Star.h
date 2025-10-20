//==============================================================================
// 
//  Project: mm2hack
//  Star.h
// 
//  Improved star effect abstract class.
// 
//==============================================================================
#pragma once

#include <string_view>
#include "StarState.h"

namespace mm2hack::apps::vfx::stareffects
{
    // Using in Star, FixedStar, BgStarField classes
    inline constexpr std::wstring_view kStarSpriteName = L"STARS";

    // Abstract class representing a star effect
    class Star
    {
    public:
        Star(int typeIndex, float x, float y, float vx, float vy);
        Star(const StarState& s)
            : _typeIndex(s.type), _x(s.x), _y(s.y), _vx(s.vx), _vy(s.vy)
        {
        }
        virtual ~Star() = default;

        virtual void Update();
        virtual void Draw();
        virtual bool IsOffScreen() const;
        StarState ToState() const;

    protected:
        int _typeIndex;     // 0: Flashing, 1: Bright, 2: Dim
        float _x, _y;
        float _vx, _vy;
    };
}