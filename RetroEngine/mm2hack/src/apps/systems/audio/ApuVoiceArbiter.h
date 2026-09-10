//==============================================================================
//
//  Project: mm2hack
//  ApuVoiceArbiter.h
//
//  Arbitrates exclusive SE ownership of the five logical NES APU voices.
//
//==============================================================================
#pragma once

#include <array>
#include <optional>
#include <string>
#include <vector>

#include "ApuVoice.h"
#include "SePriority.h"

namespace mm2hack::apps::systems::audio
{
    /// A requested logical voice and the priority used to compete for it.
    struct ApuVoiceClaim
    {
        ApuVoice voice = ApuVoice::Pulse1;
        SePriority priority = SePriority::Normal;
    };

    /// The active SE owner of one logical APU voice.
    struct ApuVoiceOwner
    {
        std::wstring name;
        SePriority priority = SePriority::Normal;
    };

    /// Result of one atomic acquisition attempt.
    struct ApuVoiceAcquisition
    {
        bool accepted = false;
        std::vector<std::wstring> displacedOwners;
    };

    /// Maintains exclusive SE ownership across the five logical APU voices.
    class ApuVoiceArbiter
    {
    public:
        /// Acquires every requested voice or leaves all ownership unchanged.
        ApuVoiceAcquisition Acquire(
            const std::wstring& owner,
            const std::vector<ApuVoiceClaim>& claims);
        /// Releases every voice held by the named SE.
        void Release(const std::wstring& owner);
        /// Releases one voice when its SE stem finishes naturally.
        void Release(ApuVoice voice, const std::wstring& owner);
        /// Releases every voice.
        void Clear() noexcept;

        /// Returns the SE owner of a voice, or nullptr when BGM may be audible.
        const ApuVoiceOwner* GetOwner(ApuVoice voice) const noexcept;
        /// Reports whether an SE currently preempts the voice.
        bool IsOwned(ApuVoice voice) const noexcept;
        /// Returns the highest priority among active SE voice owners.
        SePriority GetCurrentMaxPriority() const noexcept;

    private:
        std::array<std::optional<ApuVoiceOwner>, kApuVoiceCount> _owners{};
    };
}
