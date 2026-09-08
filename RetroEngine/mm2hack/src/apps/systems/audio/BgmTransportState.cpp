#include "pch.h"

#include "BgmTransportState.h"

#include <array>
#include <cmath>
#include <cstddef>

#include "ApuVoice.h"
#include "config/SystemConfig.h"

namespace mm2hack::apps::systems::audio
{
    bool BgmVoiceTransportState::IsValid() const noexcept
    {
        return ToIndex(voice) < kApuVoiceCount && position_milliseconds >= 0 &&
            position_milliseconds <= kMaxPositionMilliseconds &&
            logical_volume >= 0 && logical_volume <= config::SystemConfig::kAudioMaxVolume;
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
