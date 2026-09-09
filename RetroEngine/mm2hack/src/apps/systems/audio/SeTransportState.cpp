#include "pch.h"

#include "SeTransportState.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>

#include "ApuVoice.h"
#include "config/SystemConfig.h"
#include "core/save/StateIO.h"

namespace mm2hack::apps::systems::audio
{
    namespace
    {
        constexpr std::uint16_t kSeTransportStateVersion = 1;
    }

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

    bool SeTransportState::Save(core::save::StateWriter& writer) const
    {
        if (!IsValid() ||
            continuous_instances.size() > (std::numeric_limits<std::uint8_t>::max)())
        {
            return false;
        }
        if (!writer.WriteU16(kSeTransportStateVersion) ||
            !writer.WriteI32(master_volume) ||
            !writer.WriteU8(static_cast<std::uint8_t>(continuous_instances.size())))
        {
            return false;
        }

        for (const auto& instance : continuous_instances)
        {
            if (!writer.WriteWString(
                    instance.name,
                    static_cast<std::uint16_t>(ContinuousSeTransportState::kMaxNameLength)) ||
                !writer.WriteU8(static_cast<std::uint8_t>(instance.playback_status)) ||
                !writer.WriteU8(static_cast<std::uint8_t>(instance.voices.size())))
            {
                return false;
            }
            for (const auto& voice_state : instance.voices)
            {
                if (!writer.WriteU8(static_cast<std::uint8_t>(voice_state.voice)) ||
                    !writer.WriteU64(
                        static_cast<std::uint64_t>(voice_state.position_milliseconds)) ||
                    !writer.WriteI32(voice_state.logical_volume))
                {
                    return false;
                }
            }
        }
        return true;
    }

    bool SeTransportState::Load(core::save::StateReader& reader)
    {
        SeTransportState loaded{};
        std::uint16_t version{};
        std::uint8_t instance_count{};
        if (!reader.ReadU16(version) || version != kSeTransportStateVersion ||
            !reader.ReadI32(loaded.master_volume) ||
            !reader.ReadU8(instance_count) || instance_count > kApuVoiceCount)
        {
            return false;
        }

        loaded.continuous_instances.reserve(instance_count);
        for (std::uint8_t instance_index = 0;
            instance_index < instance_count; ++instance_index)
        {
            ContinuousSeTransportState instance{};
            std::uint8_t playback_status_value{};
            std::uint8_t voice_count{};
            if (!reader.ReadWString(
                    instance.name,
                    static_cast<std::uint16_t>(ContinuousSeTransportState::kMaxNameLength)) ||
                !reader.ReadU8(playback_status_value) ||
                !reader.ReadU8(voice_count) || voice_count > kApuVoiceCount)
            {
                return false;
            }
            instance.playback_status = static_cast<SePlaybackStatus>(playback_status_value);
            instance.voices.reserve(voice_count);
            for (std::uint8_t voice_index = 0; voice_index < voice_count; ++voice_index)
            {
                SeVoiceTransportState voice_state{};
                std::uint8_t voice_value{};
                std::uint64_t position{};
                if (!reader.ReadU8(voice_value) ||
                    !reader.ReadU64(position) ||
                    position > static_cast<std::uint64_t>(
                        (std::numeric_limits<std::int64_t>::max)()) ||
                    !reader.ReadI32(voice_state.logical_volume))
                {
                    return false;
                }
                voice_state.voice = static_cast<ApuVoice>(voice_value);
                voice_state.position_milliseconds = static_cast<std::int64_t>(position);
                instance.voices.push_back(voice_state);
            }
            loaded.continuous_instances.push_back(std::move(instance));
        }

        if (!loaded.IsValid()) return false;
        *this = std::move(loaded);
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
