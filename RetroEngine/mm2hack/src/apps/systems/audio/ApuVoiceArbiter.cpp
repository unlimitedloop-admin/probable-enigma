#include "pch.h"

#include "ApuVoiceArbiter.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "ApuVoice.h"
#include "SePriority.h"

namespace mm2hack::apps::systems::audio
{
    ApuVoiceAcquisition ApuVoiceArbiter::Acquire(
        const std::wstring& owner,
        const std::vector<ApuVoiceClaim>& claims)
    {
        ApuVoiceAcquisition result{};
        if (owner.empty() || claims.empty())
        {
            return result;
        }

        std::array<bool, kApuVoiceCount> requested{};
        for (const auto& claim : claims)
        {
            const std::size_t index = ToIndex(claim.voice);
            if (index >= requested.size() || requested[index])
            {
                return result;
            }
            requested[index] = true;

            const auto& current = _owners[index];
            if (current.has_value() && claim.priority < current->priority)
            {
                return result;
            }
        }

        for (std::size_t index = 0; index < _owners.size(); ++index)
        {
            const auto& current = _owners[index];
            if (!current.has_value())
            {
                continue;
            }
            if ((current->name == owner || requested[index]) &&
                std::find(result.displacedOwners.begin(), result.displacedOwners.end(), current->name) ==
                result.displacedOwners.end())
            {
                result.displacedOwners.push_back(current->name);
            }
        }

        for (auto& current : _owners)
        {
            if (!current.has_value())
            {
                continue;
            }
            if (std::find(result.displacedOwners.begin(), result.displacedOwners.end(), current->name) !=
                result.displacedOwners.end())
            {
                current.reset();
            }
        }
        for (const auto& claim : claims)
        {
            _owners[ToIndex(claim.voice)] = ApuVoiceOwner{ owner, claim.priority };
        }

        result.accepted = true;
        return result;
    }

    void ApuVoiceArbiter::Release(const std::wstring& owner)
    {
        for (auto& current : _owners)
        {
            if (current.has_value() && current->name == owner)
            {
                current.reset();
            }
        }
    }

    void ApuVoiceArbiter::Release(ApuVoice voice, const std::wstring& owner)
    {
        const std::size_t index = ToIndex(voice);
        if (index >= _owners.size()) return;
        auto& current = _owners[index];
        if (current.has_value() && current->name == owner)
        {
            current.reset();
        }
    }

    void ApuVoiceArbiter::Clear() noexcept
    {
        for (auto& current : _owners)
        {
            current.reset();
        }
    }

    const ApuVoiceOwner* ApuVoiceArbiter::GetOwner(ApuVoice voice) const noexcept
    {
        const std::size_t index = ToIndex(voice);
        if (index >= _owners.size()) return nullptr;
        const auto& current = _owners[index];
        return current.has_value() ? &current.value() : nullptr;
    }

    bool ApuVoiceArbiter::IsOwned(ApuVoice voice) const noexcept
    {
        const std::size_t index = ToIndex(voice);
        return index < _owners.size() && _owners[index].has_value();
    }

    SePriority ApuVoiceArbiter::GetCurrentMaxPriority() const noexcept
    {
        SePriority result = SePriority::Low;
        for (const auto& current : _owners)
        {
            if (current.has_value())
            {
                result = (std::max)(result, current->priority);
            }
        }
        return result;
    }
}
