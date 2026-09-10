//==============================================================================
// 
//  Project: mm2hack
//  StarState.h
// 
//  State of the star effects.
// 
//==============================================================================
#pragma once

#include <cstdint>

#include "core/save/StateIO.h"

namespace mm2hack::apps::vfx::stareffects
{
    // Star effect state structure
    struct StarState
    {
        std::int32_t type{};
        float x{};
        float y{};
        float vx{};
        float vy{};

        bool Save(core::save::StateWriter& writer) const
        {
            return writer.WriteI32(type) &&
                writer.WriteF32(x) && writer.WriteF32(y) &&
                writer.WriteF32(vx) && writer.WriteF32(vy);
        }

        bool Load(core::save::StateReader& reader)
        {
            return reader.ReadI32(type) &&
                reader.ReadF32(x) && reader.ReadF32(y) &&
                reader.ReadF32(vx) && reader.ReadF32(vy);
        }
    };

    struct FixedStarState
    {
        std::int32_t tileIndex{};
        float x{};
        float y{};

        bool Save(core::save::StateWriter& writer) const
        {
            return writer.WriteI32(tileIndex) &&
                writer.WriteF32(x) && writer.WriteF32(y);
        }
        bool Load(core::save::StateReader& reader)
        {
            return reader.ReadI32(tileIndex) &&
                reader.ReadF32(x) && reader.ReadF32(y);
        }
    };
}
