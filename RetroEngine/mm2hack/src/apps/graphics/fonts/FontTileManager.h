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
#include <vector>

namespace mm2hack::apps::graphics::fonts
{
    class FontTileManager final
    {
    public:
        FontTileManager() = default;
        ~FontTileManager();

        // JSONパスのみ（PNGはJSONに記載 or JSONと同名.png を推測）
        void Load(const std::wstring& name, std::wstring_view jsonPath);
        // PNG/JSONを別々に指定（PNG優先）
        void Load(const std::wstring& name, std::wstring_view pngPath, std::wstring_view jsonPath);
        void Remove(const std::wstring& name);

        void DrawTextImage(const std::wstring& text, int x, int y, int spacing = 8) const;
        void ChangeColoredImage(const std::wstring& setName, char ch, uint8_t r, uint8_t g, uint8_t b);

        void SetUp();
        void ShutDown();

        // --- FadeIO hooks ---
        void SetGlobalVariant(int v) noexcept { _globalVariant = v; }
        [[nodiscard]] int GlobalVariant() const noexcept { return _globalVariant; }
        [[nodiscard]] int MaxVariant() const noexcept;              // sets の最小 (variantCount-1)
        void SetGlobalVariantClamped(int v) noexcept;               // 0..Max に丸めて設定
        [[nodiscard]] int VariantCountByName(const std::wstring& setName) const; // セット単位の段数

    private:
        struct FontSet
        {
            int softImage{ -1 };
            int tile_w{ 8 };
            int tile_h{ 8 };
            int tiles_x{ 16 };
            int tiles_y{ 2 };
            int variantCount{ 1 };
            std::map<char, int> charToIndex;                       // char -> tile index
            std::vector<std::map<char, int>> graphsByVariant;      // [variant] : char -> graph handle
        };

        std::map<std::wstring, FontSet> _fontSets; // setName -> FontSet
        int _globalVariant{ 0 };

        // JSONを解釈してメタ情報を返す
        struct ParsedMeta
        {
            std::wstring pngPath;  // JSON記述 or JSONと同名 .png 推測
            int tile_w{ 8 }, tile_h{ 8 }, tiles_x{ 16 }, tiles_y{ 2 };
            int variant_count{ 1 };
            int nes_fade_step{ 16 };
            std::map<char, int> charToIndex; // 任意（無ければデフォルト生成）
        };

        static ParsedMeta ParseMeta_(const std::wstring& jsonPath, const std::wstring& setName);
        static std::wstring DerivePngFromJsonPath_(const std::wstring& jsonPath);
        static std::wstring ConcatPath_(const std::filesystem::path& baseDir, const std::wstring& rel);

        void CreateFontGraphs_(const std::wstring& name, int softImage,
                               int tile_w, int tile_h, int tiles_x, int tiles_y,
                               const std::map<char, int>& charIndexMap,
                               int variant_count, int fade_step);
    };
}