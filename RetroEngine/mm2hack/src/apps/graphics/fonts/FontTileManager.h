//==============================================================================
// 
//  Project: mm2hack
//  FontTileManager.h
// 
//  Specify a char strings to draw an NES style bitmap fonts.
// 
//==============================================================================
#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <string_view>

namespace mm2hack::apps::graphics::fonts
{
    // FontTileManager manages bitmap fonts loaded from PNG files
    class FontTileManager final
    {
    public:
        FontTileManager() = default;
        ~FontTileManager();

        // Load a font set from a JSON file specifying metadata and PNG path
        void Load(const std::wstring& name, std::wstring_view pngPath, std::wstring_view jsonPath);
        // Load a font set from a JSON file specifying metadata and PNG path (alias)
        void Load(const std::wstring& name, std::wstring_view jsonPath);
        // Remove a loaded font set by name
        void Remove(const std::wstring& name);
        // Draw text using the loaded font sets at specified coordinates with optional spacing
        void DrawTextImage(const std::wstring& text, int x, int y, int spacing = 8) const;
        // Change the color of a specific character in a font set
        void ChangeColoredImage(const std::wstring& setName, char ch, uint8_t r, uint8_t g, uint8_t b);

        // Supports standard font bitmap installation (using alphabet and numbers)
        void SetUp();
        // Release all loaded font sets and associated resources
        void Shutdown();

    private:
        struct FontSet
        {
            int softImage{ -1 };
            int tile_w{ 8 };
            int tile_h{ 8 };
            int tiles_x{ 16 };
            int tiles_y{ 2 };
            std::map<char, int> graphs;       // char -> graph handle
            std::map<char, int> charToIndex;  // char -> tile index (0..tiles_x*tiles_y-1)
        };

        // Parse JSON and return metadata
        struct ParsedMeta
        {
            std::wstring pngPath;
            int tile_w{ 8 }, tile_h{ 8 }, tiles_x{ 16 }, tiles_y{ 2 };
            std::map<char, int> charToIndex;
        };

        static ParsedMeta ParseMeta_(const std::wstring& jsonPath, const std::wstring& setName);
        static std::wstring DerivePngFromJsonPath_(const std::wstring& jsonPath);
        static std::wstring ConcatPath_(const std::filesystem::path& baseDir, const std::wstring& rel);

        void CreateFontGraphs_(const std::wstring& name, int softImage,
            int tile_w, int tile_h, int tiles_x, int tiles_y,
            const std::map<char, int>& charIndexMap);

    private:
        const std::wstring kClassName = L"FontTileManager";

        std::map<std::wstring, FontSet> _fontSets;          // font data by name
    };
}