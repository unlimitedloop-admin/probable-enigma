//==============================================================================
// 
//  Project: mm2hack
//  PhaseResult.h
// 
//  Return value that conveys the phase change request from a Phase to its Scene.
// 
//==============================================================================
#pragma once

#include <string>

namespace mm2hack::apps::resources::parameters
{
    class Parameters;
}

namespace mm2hack::apps::scenes::phases
{
    enum class PhaseResultKind
    {
        None,
        RequestTransition,
    };

    // Result of a Phase update, indicating whether a transition is requested
    struct PhaseResult final
    {
        PhaseResultKind kind{ PhaseResultKind::None };  // Type of result

        // Scene interprets this key: e.g., L"TopMenu", L"AreaA", L"AreaB", L"Clear", L"GameOver", etc.
        std::wstring next_key{};

        // Optional payload (non-owning). Keep ownership on the Phase (or Scene) side.
        const resources::parameters::Parameters* params{ nullptr };

        [[nodiscard]] bool HasRequest() const noexcept
        {
            return kind == PhaseResultKind::RequestTransition;
        }

        static PhaseResult None() noexcept
        {
            return {};
        }

        static PhaseResult Transition(const std::wstring& key, const resources::parameters::Parameters* p = nullptr)
        {
            PhaseResult r{};
            r.kind = PhaseResultKind::RequestTransition;
            r.next_key = key;
            r.params = p;
            return r;
        }
    };
}