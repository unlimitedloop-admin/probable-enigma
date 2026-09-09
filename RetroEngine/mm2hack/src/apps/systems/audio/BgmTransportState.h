//==============================================================================
//
//  Project: mm2hack
//  BgmTransportState.h
//
//  Logical, backend-independent BGM transport state.
//
//==============================================================================
#pragma once

#include <cstddef>
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
    /// Logical BGM transport mode independent of backend playback handles.
    enum class BgmPlaybackStatus : std::uint8_t
    {
        Stopped,
        Playing,
        Paused
    };

    /// Saved transport and logical volume for one stable APU voice.
    struct BgmVoiceTransportState
    {
        static constexpr std::int64_t kMaxPositionMilliseconds =
            24LL * 60LL * 60LL * 1000LL;

        ApuVoice voice = ApuVoice::Pulse1;
        std::int64_t position_milliseconds = 0;
        int logical_volume = 0;

        /// Validates bounds before the state reaches an audio backend.
        bool IsValid() const noexcept;
    };

    /// Complete logical transport state for one registered multi-stem BGM.
    struct BgmTransportState
    {
        static constexpr std::size_t kMaxTrackNameLength = 128;
        static constexpr int kMaxFadeFrames = 60 * 60 * 10;

        std::wstring track_name;
        BgmPlaybackStatus playback_status = BgmPlaybackStatus::Stopped;
        std::vector<BgmVoiceTransportState> voices;
        int master_volume = 0;
        double loop_start_seconds = 0.0;
        double loop_end_seconds = 0.0;
        bool is_fading = false;
        int fade_target = 0;
        int fade_step = 0;
        int fade_frames_remaining = 0;

        /// Serializes a versioned logical snapshot without backend handles.
        bool Save(core::save::StateWriter& writer) const;
        /// Loads transactionally, preserving this object when decoding fails.
        bool Load(core::save::StateReader& reader);
        /// Performs resource-independent structural and bounds validation.
        bool IsValid() const noexcept;
    };
}
