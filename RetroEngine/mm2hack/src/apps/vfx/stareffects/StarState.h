//==============================================================================
// 
//  Project: mm2hack
//  StarState.h
// 
//  State of the star effects.
// 
//==============================================================================
#pragma once

#include <iostream>
#include <istream>

namespace mm2hack::apps::vfx::stareffects
{
    // Star effect state structure
    struct StarState
    {
        int type;
        float x, y;
        float vx, vy;

        void Save(std::ostream& out) const
        {
            out.write(reinterpret_cast<const char*>(&type), sizeof(type));
            out.write(reinterpret_cast<const char*>(&x), sizeof(x));
            out.write(reinterpret_cast<const char*>(&y), sizeof(y));
            out.write(reinterpret_cast<const char*>(&vx), sizeof(vx));
            out.write(reinterpret_cast<const char*>(&vy), sizeof(vy));
        }

        void Load(std::istream& in)
        {
            in.read(reinterpret_cast<char*>(&type), sizeof(type));
            in.read(reinterpret_cast<char*>(&x), sizeof(x));
            in.read(reinterpret_cast<char*>(&y), sizeof(y));
            in.read(reinterpret_cast<char*>(&vx), sizeof(vx));
            in.read(reinterpret_cast<char*>(&vy), sizeof(vy));
        }
    };

    struct FixedStarState
    {
        int tileIndex;
        float x, y;

        void Save(std::ostream& out) const
        {
            out.write(reinterpret_cast<const char*>(&tileIndex), sizeof(tileIndex));
            out.write(reinterpret_cast<const char*>(&x), sizeof(x));
            out.write(reinterpret_cast<const char*>(&y), sizeof(y));
        }
        void Load(std::istream& in)
        {
            in.read(reinterpret_cast<char*>(&tileIndex), sizeof(tileIndex));
            in.read(reinterpret_cast<char*>(&x), sizeof(x));
            in.read(reinterpret_cast<char*>(&y), sizeof(y));
        }
    };
}