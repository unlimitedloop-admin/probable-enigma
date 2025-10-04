//==============================================================================
// 
//  Project: mm2hack
//  SpriteAtlas.h
// 
//  Drawable and manageable sprite atlas.
// 
//==============================================================================
#pragma once

#include <string>
#include <vector>

// Forward declare to avoid header include of DxLib in all translation units
struct tagSOFTIMAGE; // DxLib's SoftImage opaque type (we only hold handle int)

namespace mm2hack::apps::graphics::sprite
{
    // A sprite atlas is a collection of related sprites (frames) arranged in a grid
    class SpriteAtlas final
    {
    public:
        struct DivSettings
        {
            int tile_w{ 0 };
            int tile_h{ 0 };
            int tiles_x{ 0 };
            int tiles_y{ 0 };
        };

        // Optional palette/variant settings. If variant_count == 1, palette is not used.
        struct PaletteConfig
        {
            int variant_count{ 1 };     // e.g., 4 for NES fade steps
            int nes_fade_step{ 16 };    // +16/-16 offset rule
        };

        SpriteAtlas(std::wstring name, DivSettings div,
                    int soft_image_handle, std::vector<std::vector<int>> graphs_by_variant) noexcept;
        ~SpriteAtlas();
        SpriteAtlas(const SpriteAtlas&) = delete;
        SpriteAtlas& operator=(const SpriteAtlas&) = delete;
        SpriteAtlas(SpriteAtlas&& other) noexcept;
        SpriteAtlas& operator=(SpriteAtlas&& other) noexcept;

        // Properties
        [[nodiscard]] const std::wstring& Name() const noexcept { return _name; }
        [[nodiscard]] DivSettings GetDiv() const noexcept { return _div; }
        [[nodiscard]] int VariantCount() const noexcept { return static_cast<int>(_graphs_by_variant.size()); }
        [[nodiscard]] int FramesPerVariant() const noexcept;

        // Draw specified frame with specified color-variant
        void Draw(int variant, int frame, int x, int y) const noexcept;

        // Replace a color in the palette for all variants
        bool ReplacePaletteColorIndex(int variant, int targetPaletteIndex, int sourcePaletteIndex) noexcept;
        // Replace a color in the palette for all variants (RGB match)
        bool ReplacePaletteColorRGB(int variant, unsigned char r, unsigned char g, unsigned char b, int sourcePaletteIndex) noexcept;
        // Apply a random hue shift to the specified variant
        bool ApplyRandomHueToVariant(int variant) noexcept;
        // Apply HSB adjustments to the specified variant
        bool ApplyHSBToVariant(int variant, int hueAdd, int satAdd, int briAdd) noexcept;

    private:
        bool RebuildVariantFromSoftImage_(int variant) noexcept;    // rebuild graphs for the variant from SoftImage
        void Dispose() noexcept;    // release SoftImage and graphs

    private:
        std::wstring _name{};       // unique name identifier
        DivSettings _div{};         // division settings
        int _soft_image{ -1 };      // keep if needed (palette rebuild), otherwise -1
        std::vector<std::vector<int>> _graphs_by_variant;   // [variant][frame] -> graph handle, variant for palette swaps
    };
}