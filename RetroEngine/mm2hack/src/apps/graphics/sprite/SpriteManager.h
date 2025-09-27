//==============================================================================
// 
//  Project: mm2hack
//  SpriteManager.h
// 
//  Manages sprite textures and their properties.
// 
//==============================================================================
#pragma once

#include <limits>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include "SpriteCatalog.h"

namespace mm2hack::apps::graphics::sprite
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
        // Compatibility: draw by name (internally cached after first use)
        void Use(const std::wstring& name, int frame, int x, int y);

        // Variant (palette step) controls
        void SetGlobalVariant(int variant) noexcept { _global_variant = variant; }
        [[nodiscard]] int GlobalVariant() const noexcept { return _global_variant; }

        // Palette color replacement (for NES-style palette swaps)
        bool ReplacePaletteColorByName(const std::wstring& name, int targetPaletteIndex, int sourcePaletteIndex, int variant = 0);
        bool ReplacePaletteColorById(Id id, int targetPaletteIndex, int sourcePaletteIndex, int variant = 0);
        bool ApplyRandomColorFilterByName(const std::wstring& name, int variant = 0);
        bool ApplyRandomColorFilterById(Id id, int variant = 0);
        bool ApplyHSBFilterById(Id id, int variant, int hueAdd, int satAdd, int briAdd);

        // Event monitoring
        void SetEvents(SpriteCatalog::Events events) { _catalog.SetEvents(std::move(events)); }

        // Utilities
        [[nodiscard]] bool Has(const std::wstring& name) const { return _catalog.Has(name); }
        [[nodiscard]] Id GetId(const std::wstring& name) const { return _catalog.GetId(name); }

        // --- Release / Remove APIs ---
        void ReleaseById(Id id);
        void ReleaseByName(const std::wstring& name);
        void ReleaseAll();

    private:
        Id CacheId_(const std::wstring& name);      // Get cached Id or cache it if not found

    private:
        static constexpr Id kInvalidId = std::numeric_limits<Id>::max();
        SpriteCatalog _catalog{};
        mutable std::unordered_map<std::wstring, Id> _name_cache{};
        int _global_variant{ 0 };
    };
}