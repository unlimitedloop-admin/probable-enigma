#include "pch.h"

#include "SeTransportState.h"

#include <array>
#include <cstddef>

#include "ApuVoice.h"
#include "config/SystemConfig.h"

namespace mm2hack::apps::systems::audio
{
    bool SeVoiceTransportState::IsValid() const noexcept
    {
        return ToIndex(voice) < kApuVoiceCount && position_milliseconds >= 0 &&
            position_milliseconds <= kMaxPositionMilliseconds &&
            logical_volume >= 0 && logical_volume <= config::SystemConfig::kAudioMaxVolume;
    }

    bool ContinuousSeTransportState::IsValid() const noexcept
    {
        if (name.empty() || name.size() > kMaxNameLength || voices.empty() ||
            voices.size() > kApuVoiceCount ||
            (playback_status != SePlaybackStatus::Playing &&
                playback_status != SePlaybackStatus::Paused))
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

    bool SeTransportState::IsValid() const noexcept
    {
        if (master_volume < 0 || master_volume > config::SystemConfig::kAudioMaxVolume ||
            continuous_instances.size() > kApuVoiceCount)
        {
            return false;
        }

        std::array<bool, kApuVoiceCount> occupied{};
        for (std::size_t instance_index = 0;
            instance_index < continuous_instances.size(); ++instance_index)
        {
            const auto& instance = continuous_instances[instance_index];
            if (!instance.IsValid()) return false;
            for (std::size_t earlier = 0; earlier < instance_index; ++earlier)
            {
                if (continuous_instances[earlier].name == instance.name) return false;
            }
            for (const auto& voice_state : instance.voices)
            {
                const std::size_t index = ToIndex(voice_state.voice);
                if (occupied[index]) return false;
                occupied[index] = true;
            }
        }
        return true;
    }
}
