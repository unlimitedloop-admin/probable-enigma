//==============================================================================
// 
//  Project: mm2hack
//  SpriteAtlas.h
// 
//  Drawable and manageable sprite atlas.
// 
//==============================================================================
#pragma once

#include <span>
#include <string>
#include <vector>
#include "apps/rendering/common/DividedGraphLoader.h"

// Forward declare to avoid header include of DxLib in all translation units
struct tagSOFTIMAGE; // DxLib's SoftImage opaque type (we only hold handle int)

namespace mm2hack::apps::rendering::sprite
{
    // A sprite atlas is a collection of related sprites (frames) arranged in a grid
    class SpriteAtlas final
    {
    public:
        using DivSettings = common::DivSettings;

        struct PaletteColorMapping
        {
            int source_palette_index{ 0 };
            int target_palette_index{ 0 };
        };

        SpriteAtlas(DivSettings div,
                    int soft_image_handle, std::vector<std::vector<int>> graphs_by_variant) noexcept;
        ~SpriteAtlas();
        SpriteAtlas(const SpriteAtlas&) = delete;
        SpriteAtlas& operator=(const SpriteAtlas&) = delete;
        SpriteAtlas(SpriteAtlas&& other) noexcept;
        SpriteAtlas& operator=(SpriteAtlas&& other) noexcept;

        // Properties
        [[nodiscard]] int VariantCount() const noexcept { return static_cast<int>(_graphs_by_variant.size()); }

        // Draw specified frame with specified color-variant
        void Draw(int variant, int frame, int x, int y) const noexcept;

        // Replace a color in the palette for all variants
        bool ReplacePaletteColorIndex(int variant, int targetPaletteIndex, int sourcePaletteIndex) noexcept;
        // Replace a color in the palette for all variants (RGB match)
        bool ReplacePaletteColorRGB(int variant, unsigned char r, unsigned char g, unsigned char b, int sourcePaletteIndex) noexcept;
        // Replace indexed palette entries and rebuild every fade variant.
        bool ReplacePaletteColors(
            std::span<const PaletteColorMapping> mappings) noexcept;
        // Replace pixels matching one RGB color while preserving their alpha values.
        bool ReplacePixelColors(
            int variant,
            std::span<const PaletteColorMapping> mappings) noexcept;
        // Apply HSB adjustments to the specified variant
        bool ApplyHSBToVariant(int variant, int hueAdd, int satAdd, int briAdd) noexcept;

    private:
        bool rebuildVariantFromSoftImage_(int variant) noexcept;    // rebuild graphs for the variant from SoftImage
        void dispose_() noexcept;    // release SoftImage and graphs

    private:
        const std::wstring kClassName{ L"SpriteAtlas" };

        DivSettings _div{};         // division settings
        int _soft_image{ -1 };      // keep if needed (palette rebuild), otherwise -1
        std::vector<std::vector<int>> _graphs_by_variant;   // [variant][frame] -> graph handle, variant for palette swaps
    };
}
