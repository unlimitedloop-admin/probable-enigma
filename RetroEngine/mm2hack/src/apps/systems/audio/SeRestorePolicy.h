//==============================================================================
//
//  Project: mm2hack
//  SeRestorePolicy.h
//
//  Save-state restoration policy for sound effects.
//
//==============================================================================
#pragma once

#include <cstdint>

namespace mm2hack::apps::systems::audio
{
    /// Controls whether an active SE is discarded or reconstructed on load.
    enum class SeRestorePolicy : std::uint8_t
    {
        Transient,
        Continuous
    };
}
