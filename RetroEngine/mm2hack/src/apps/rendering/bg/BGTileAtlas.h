//==============================================================================
// 
//  Project: mm2hack
//  BGTileAtlas.h
// 
//  Mapping and drawing BG tiles.
// 
//==============================================================================
#pragma once

#include <span>
#include <string>
#include <unordered_map>
#include <vector>
#include "apps/rendering/common/DividedGraphLoader.h"
#include "BGTilePalette.h"

namespace mm2hack::apps::rendering::bg
{
    // Tile atlas for background graphics
    class BGTileAtlas final
    {
    public:
        using DivSettings = common::DivSettings;

        BGTileAtlas(std::wstring name,
            DivSettings div,
            int soft_image_handle,
            std::vector<std::vector<int>> graphs_by_variant) noexcept;
        ~BGTileAtlas();
        BGTileAtlas(const BGTileAtlas&) = delete;
        BGTileAtlas& operator=(const BGTileAtlas&) = delete;
        BGTileAtlas(BGTileAtlas&&) noexcept;
        BGTileAtlas& operator=(BGTileAtlas&&) noexcept;

        // Properties
        [[nodiscard]] const std::wstring& Name() const noexcept { return _name; }
        [[nodiscard]] int VariantCount() const noexcept { return static_cast<int>(_graphs_by_variant.size()); }
        [[nodiscard]] int TilesPerVariant() const noexcept;

        // draw tile index in the atlas (0..tiles_x*tiles_y-1)
        void DrawTile(int variant, int tile_index, int x, int y) const noexcept;
        // Creates a palette variant for a specific tile -- one graph per fade
        // step (VariantCount()), so the recolored tile fades with the rest of
        // the BG. Returns the new palette variant's index, or -1 on failure.
        [[nodiscard]] int CreateTilePaletteVariant(int tile_index, std::span<const BGPaletteColorMapping> mappings);
        // Draws a specific tile using its local palette variant, at fade step
        // `fade_variant` (the same step DrawTile() takes as `variant`).
        void DrawTilePaletteVariant(int tile_index, int palette_variant, int fade_variant, int x, int y) const noexcept;

    private:
        void dispose_() noexcept;

    private:
        const std::wstring kClassName{ L"BGTileAtlas" };

        std::wstring _name{};
        DivSettings _div{};
        int _soft_image{ -1 };
        std::vector<std::vector<int>> _graphs_by_variant;   // [variant][tile_index]

        // [tile_index] -> [palette_variant][fade_variant] -> graph handle
        std::unordered_map<int, std::vector<std::vector<int>>> _tile_palette_variants{};
    };
}
