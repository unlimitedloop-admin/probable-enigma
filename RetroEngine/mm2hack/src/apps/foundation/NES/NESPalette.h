//==============================================================================
// 
//  Project: mm2hack
//  NESPalette.h
// 
//  Utility class for loading and using NES 64-color background palette.
// 
//==============================================================================
#pragma once

#include <array>
#include <string>

namespace mm2hack::apps::foundation::NES
{
    // NESPalette class for loading and using NES 64-color background palette
    class NESPalette final
    {
    public:
        struct RGB
        {
            int red;
            int green;
            int blue;
        };

        NESPalette() = delete;
        NESPalette(const NESPalette&) = delete;
        NESPalette& operator=(const NESPalette&) = delete;
        NESPalette(NESPalette&&) = delete;
        NESPalette& operator=(NESPalette&&) = delete;
        ~NESPalette() = delete;
        // This class is not copyable or movable (static member defined only)

        // Load palette data from external text file
        static bool LoadPaletteFromFile(const std::wstring& file_path);
        // Set background color using palette number
        static void SetBackgroundFor(size_t palette_no);
        // Get RGB color from palette number
        static const RGB& GetColor(size_t index);

    private:
        static inline std::array<RGB, 64> _palette_data{};      // NES 64-color background palette data
    };
}