//==============================================================================
// 
//  Project: mm2hack
//  SpriteManager.h
// 
//  Manages sprite textures and their properties.
// 
//==============================================================================
#pragma once

#include <span>
#include <string>
#include <string_view>
#include "SpriteAtlas.h"
#include "SpriteCatalog.h"

namespace mm2hack::apps::rendering::sprite
{
    // Top-level sprite manager that uses SpriteCatalog for loading and caching
    class SpriteManager
    {
    public:
        using Id = SpriteCatalog::Id;

        SpriteManager() = default;
        ~SpriteManager() = default;

        // Load a sprite atlas from PNG + JSON metadata (div settings, optional palette variants)
        // The asset files are defined in config/AssetPortfolio.def
        Id Load(const std::wstring& name, const std::wstring_view png_path, const std::wstring_view json_path);
        // Fast path: draw sprite by Id (O(1))
        void UseById(Id id, int frame, int x, int y) const noexcept;
        // Fast path: draw sprite by Id (Per-call variant override)
        void UseByIdVariant(Id id, int variant, int frame, int x, int y) const noexcept;
        // Palette color replacement (for NES-style palette swaps)
        bool ReplacePaletteColorById(Id id, int targetPaletteIndex, int sourcePaletteIndex, int variant = 0);
        bool ReplacePixelColorsById(Id id, std::span<const SpriteAtlas::PaletteColorMapping> mappings, int variant = 0);
        bool ApplyRandomColorFilterById(Id id, int variant = 0);

        // Utilities
        // Variant info
        [[nodiscard]] inline int MaxVariant() const noexcept { return _catalog.MaxVariantAcross(); }
        [[nodiscard]] int VariantCountById(Id id) const;
        void SetGlobalVariantClamped(int v) noexcept;

        // Variant (palette step) controls
        void SetGlobalVariant(int variant) noexcept { _global_variant = variant; }
        [[nodiscard]] int GlobalVariant() const noexcept { return _global_variant; }

        // --- Release / Remove APIs ---
        void ReleaseById(Id id);
        void ReleaseAll();

    private:
        const std::wstring kClassName{ L"SpriteManager" };

        SpriteCatalog _catalog{};   // underlying catalog
        int _global_variant{ 0 };   // global variant index for UseById
    };
}
