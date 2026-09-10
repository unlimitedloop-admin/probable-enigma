//==============================================================================
//
//  Project: mm2hack
//  SeTransportState.h
//
//  Logical, backend-independent continuous-SE transport state.
//
//==============================================================================
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "ApuVoice.h"

namespace mm2hack::core::save
{
    class StateReader;
    class StateWriter;
}

namespace mm2hack::apps::systems::audio
{
    enum class SePlaybackStatus : std::uint8_t
    {
        Playing,
        Paused
    };

    struct SeVoiceTransportState
    {
        static constexpr std::int64_t kMaxPositionMilliseconds =
            24LL * 60LL * 60LL * 1000LL;

        ApuVoice voice = ApuVoice::Pulse1;
        std::int64_t position_milliseconds = 0;
        int logical_volume = 0;

        bool IsValid() const noexcept;
    };

    struct ContinuousSeTransportState
    {
        static constexpr std::size_t kMaxNameLength = 128;

        std::wstring name;
        SePlaybackStatus playback_status = SePlaybackStatus::Playing;
        std::vector<SeVoiceTransportState> voices;

        bool IsValid() const noexcept;
    };

    struct SeTransportState
    {
        std::vector<ContinuousSeTransportState> continuous_instances;
        int master_volume = 0;

        /// Serializes a versioned logical snapshot without backend handles.
        bool Save(core::save::StateWriter& writer) const;
        /// Loads transactionally, preserving this object when decoding fails.
        bool Load(core::save::StateReader& reader);
        /// Performs resource-independent structural and bounds validation.
        bool IsValid() const noexcept;
    };
}
