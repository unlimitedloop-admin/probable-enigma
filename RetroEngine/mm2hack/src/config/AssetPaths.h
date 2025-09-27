//==============================================================================
// 
//  Project: mm2hack
//  AssetPaths.h
// 
//  This file contains the definitions for the asset paths used in the project.
// 
//==============================================================================
#pragma once

#include <array>
#include <string_view>

namespace mm2hack::config
{
    // ---- compile-time wide string concat (variadic) ----
    template <std::size_t... Ns>
    consteval auto wconcat_all(const wchar_t(&...parts)[Ns])
    {
        constexpr std::size_t total = (0 + ... + (Ns - 1)) + 1; // +1 for '\0'
        std::array<wchar_t, total> out{};
        std::size_t pos = 0;
        auto append = [&]<std::size_t M>(const wchar_t(&s)[M])
        {
            for (std::size_t i = 0; i < M - 1; ++i) out[pos++] = s[i];
        };
        (append(parts), ...);
        out[pos] = L'\0';
        return out;
    }

    // === base dirs (End mark '\\') ===
    inline constexpr wchar_t SpritesDir[] = LR"(assets\sprites\)";
    inline constexpr wchar_t BgsDir[]     = LR"(assets\BGs\)";
    inline constexpr wchar_t MapDataDir[] = LR"(assets\mapdata\)";
    inline constexpr wchar_t SoundsDir[]  = LR"(assets\sounds\)";
    inline constexpr wchar_t MusicDir[]   = LR"(assets\music\)";
    inline constexpr wchar_t UiDir[]      = LR"(assets\ui\)";
    inline constexpr wchar_t EffectsDir[] = LR"(assets\effects\)";
    inline constexpr wchar_t FontsDir[]   = LR"(assets\fonts\)";

    inline constexpr wchar_t ExamsDir[]   = LR"(assets\_exams\)";   // test only

#define MM2H_MAKE_WPATH(Name, Base, RelLiteral)                          \
    inline constexpr auto Name##Arr = wconcat_all(Base, RelLiteral);     \
    inline constexpr std::wstring_view Name{                             \
        Name##Arr.data(), Name##Arr.size() - 1                           \
    }
}