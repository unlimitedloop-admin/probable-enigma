//==============================================================================
//
//  Project: mm2hack
//  ApuVoice.h
//
//  Stable identities for the five logical NES APU voices.
//
//==============================================================================
#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace mm2hack::apps::systems::audio
{
    enum class ApuVoice : std::uint8_t
    {
        Pulse1,
        Pulse2,
        Triangle,
        Noise,
        Dpcm
    };

    inline constexpr std::size_t kApuVoiceCount = 5;

    constexpr std::size_t ToIndex(ApuVoice voice) noexcept
    {
        return static_cast<std::size_t>(voice);
    }

    constexpr std::string_view ToString(ApuVoice voice) noexcept
    {
        switch (voice)
        {
        case ApuVoice::Pulse1:
            return "pulse1";
        case ApuVoice::Pulse2:
            return "pulse2";
        case ApuVoice::Triangle:
            return "triangle";
        case ApuVoice::Noise:
            return "noise";
        case ApuVoice::Dpcm:
            return "dpcm";
        }
        return {};
    }

    constexpr bool TryParseApuVoice(std::string_view value, ApuVoice& voice) noexcept
    {
        if (value == "pulse1")
        {
            voice = ApuVoice::Pulse1;
            return true;
        }
        if (value == "pulse2")
        {
            voice = ApuVoice::Pulse2;
            return true;
        }
        if (value == "triangle")
        {
            voice = ApuVoice::Triangle;
            return true;
        }
        if (value == "noise")
        {
            voice = ApuVoice::Noise;
            return true;
        }
        if (value == "dpcm")
        {
            voice = ApuVoice::Dpcm;
            return true;
        }
        return false;
    }
}
