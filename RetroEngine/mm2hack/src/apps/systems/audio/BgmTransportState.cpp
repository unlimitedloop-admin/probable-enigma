#include "pch.h"

#include "BgmTransportState.h"

#include <cmath>
#include <limits>

#include "ApuVoice.h"
#include "config/SystemConfig.h"
#include "core/save/StateIO.h"

namespace mm2hack::apps::systems::audio
{
    namespace
    {
        constexpr std::uint16_t kBgmTransportStateVersion = 1;
    }

    bool BgmVoiceTransportState::IsValid() const noexcept
    {
        return ToIndex(voice) < kApuVoiceCount && position_milliseconds >= 0 &&
            position_milliseconds <= kMaxPositionMilliseconds &&
            logical_volume >= 0 && logical_volume <= config::SystemConfig::kAudioMaxVolume;
    }

    bool BgmTransportState::Save(core::save::StateWriter& writer) const
    {
        if (!IsValid() || voices.size() > (std::numeric_limits<std::uint8_t>::max)())
        {
            return false;
        }
        if (!writer.WriteU16(kBgmTransportStateVersion) ||
            !writer.WriteWString(
                track_name, static_cast<std::uint16_t>(kMaxTrackNameLength)) ||
            !writer.WriteU8(static_cast<std::uint8_t>(playback_status)) ||
            !writer.WriteU8(static_cast<std::uint8_t>(voices.size())))
        {
            return false;
        }
        for (const auto& voice_state : voices)
        {
            if (!writer.WriteU8(static_cast<std::uint8_t>(voice_state.voice)) ||
                !writer.WriteU64(static_cast<std::uint64_t>(voice_state.position_milliseconds)) ||
                !writer.WriteI32(voice_state.logical_volume))
            {
                return false;
            }
        }
        return writer.WriteI32(master_volume) &&
            writer.WriteF64(loop_start_seconds) &&
            writer.WriteF64(loop_end_seconds) &&
            writer.WriteBool(is_fading) &&
            writer.WriteI32(fade_target) &&
            writer.WriteI32(fade_step) &&
            writer.WriteI32(fade_frames_remaining);
    }

    bool BgmTransportState::Load(core::save::StateReader& reader)
    {
        BgmTransportState loaded{};
        std::uint16_t version{};
        std::uint8_t playback_status_value{};
        std::uint8_t voice_count{};
        if (!reader.ReadU16(version) || version != kBgmTransportStateVersion ||
            !reader.ReadWString(
                loaded.track_name, static_cast<std::uint16_t>(kMaxTrackNameLength)) ||
            !reader.ReadU8(playback_status_value) ||
            !reader.ReadU8(voice_count) || voice_count > kApuVoiceCount)
        {
            return false;
        }

        loaded.playback_status = static_cast<BgmPlaybackStatus>(playback_status_value);
        loaded.voices.reserve(voice_count);
        for (std::uint8_t index = 0; index < voice_count; ++index)
        {
            std::uint8_t voice_value{};
            std::uint64_t position{};
            BgmVoiceTransportState voice_state{};
            if (!reader.ReadU8(voice_value) ||
                !reader.ReadU64(position) ||
                position > static_cast<std::uint64_t>((std::numeric_limits<std::int64_t>::max)()) ||
                !reader.ReadI32(voice_state.logical_volume))
            {
                return false;
            }
            voice_state.voice = static_cast<ApuVoice>(voice_value);
            voice_state.position_milliseconds = static_cast<std::int64_t>(position);
            loaded.voices.push_back(voice_state);
        }
        if (!reader.ReadI32(loaded.master_volume) ||
            !reader.ReadF64(loaded.loop_start_seconds) ||
            !reader.ReadF64(loaded.loop_end_seconds) ||
            !reader.ReadBool(loaded.is_fading) ||
            !reader.ReadI32(loaded.fade_target) ||
            !reader.ReadI32(loaded.fade_step) ||
            !reader.ReadI32(loaded.fade_frames_remaining) ||
            !loaded.IsValid())
        {
            return false;
        }

        *this = std::move(loaded);
        return true;
    }

    bool BgmTransportState::IsValid() const noexcept
    {
        const int max_volume = config::SystemConfig::kAudioMaxVolume;
        if (master_volume < 0 || master_volume > max_volume ||
            !std::isfinite(loop_start_seconds) || !std::isfinite(loop_end_seconds) ||
            loop_start_seconds < 0.0 || loop_end_seconds < 0.0 ||
            (loop_end_seconds > 0.0 && loop_end_seconds <= loop_start_seconds) ||
            fade_target < 0 || fade_target > max_volume ||
            fade_step < -max_volume || fade_step > max_volume ||
            fade_frames_remaining < 0 || fade_frames_remaining > kMaxFadeFrames ||
            (is_fading && fade_frames_remaining == 0) ||
            (!is_fading && fade_frames_remaining != 0))
        {
            return false;
        }

        if (playback_status == BgmPlaybackStatus::Stopped)
        {
            return track_name.empty() && voices.empty() && !is_fading;
        }
        if (playback_status != BgmPlaybackStatus::Playing &&
            playback_status != BgmPlaybackStatus::Paused)
        {
            return false;
        }
        if (track_name.empty() || track_name.size() > kMaxTrackNameLength ||
            voices.empty() || voices.size() > kApuVoiceCount)
        {
            return false;
        }

        std::array<bool, kApuVoiceCount> occupied{};
        for (const auto& voice_state : voices)
        {
            if (!voice_state.IsValid()) return false;
            const std::size_t index = ToIndex(voice_state.voice);
            if (occupied[index]) return false;
            occupied[index] = true;
        }
        return true;
    }
}
