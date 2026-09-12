//==============================================================================
//
//  Project: mm2hack
//  SePriority.h
//
//  Stable priority levels used to arbitrate competing sound effects.
//
//==============================================================================
#pragma once

#include <cstdint>

namespace mm2hack::apps::systems::audio
{
    enum class SePriority : std::uint8_t
    {
        Low,
        Normal,
        High
    };
}
