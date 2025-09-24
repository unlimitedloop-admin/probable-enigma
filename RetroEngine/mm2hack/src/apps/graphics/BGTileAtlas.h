//==============================================================================
// 
//  Project: mm2hack
//  BGTileAtlas.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include <string>
#include <vector>

namespace mm2hack::apps::graphics
{
    // Tile atlas for background graphics
    class BGTileAtlas final
    {
    public:
        struct DivSettings
        {
            int tile_w{ 0 };
            int tile_h{ 0 };
            int tiles_x{ 0 };
            int tiles_y{ 0 };
        };

        struct PaletteConfig
        {
            int variant_count{ 1 }; // e.g., 4 for NES-like fade steps
            int nes_fade_step{ 16 };
        };

        BGTileAtlas(std::wstring name,
            DivSettings div,
            int soft_image_handle,
            std::vector<std::vector<int>> graphs_by_variant) noexcept;
        ~BGTileAtlas();
        BGTileAtlas(const BGTileAtlas&) = delete;
        BGTileAtlas& operator=(const BGTileAtlas&) = delete;
        BGTileAtlas(BGTileAtlas&&) noexcept;
        BGTileAtlas& operator=(BGTileAtlas&&) noexcept;

        [[nodiscard]] const std::wstring& Name() const noexcept { return _name; }
        [[nodiscard]] DivSettings GetDiv() const noexcept { return _div; }
        [[nodiscard]] int VariantCount() const noexcept { return static_cast<int>(_graphs_by_variant.size()); }
        [[nodiscard]] int TilesPerVariant() const noexcept;

        // draw tile index in the atlas (0..tiles_x*tiles_y-1)
        void DrawTile(int variant, int tile_index, int x, int y) const noexcept;

    private:
        void Dispose() noexcept;

    private:
        std::wstring _name{};
        DivSettings _div{};
        int _soft_image{ -1 };
        std::vector<std::vector<int>> _graphs_by_variant; // [variant][tile_index]
    };
}