#include "pch.h"

#include "BGTileAtlas.h"

#include <array>
#include <span>

#include "apps/foundation/NES/NESPalette.h"
#include "BGTilePalette.h"

namespace
{
    struct RGBA8
    {
        unsigned char r{ 0 };
        unsigned char g{ 0 };
        unsigned char b{ 0 };
        unsigned char a{ 255 };
    };
}

namespace mm2hack::apps::rendering::bg
{
    BGTileAtlas::BGTileAtlas(std::wstring name,
        DivSettings div,
        int soft_image_handle,
        std::vector<std::vector<int>> graphs_by_variant) noexcept
        : _name(std::move(name))
        , _div(div)
        , _soft_image(soft_image_handle)
        , _graphs_by_variant(std::move(graphs_by_variant))
    {
    }

    BGTileAtlas::~BGTileAtlas()
    {
        dispose_();
    }

    BGTileAtlas::BGTileAtlas(BGTileAtlas&& o) noexcept
        : _name(std::move(o._name))
        , _div(o._div)
        , _soft_image(o._soft_image)
        , _graphs_by_variant(std::move(o._graphs_by_variant))
    {
        o._soft_image = -1;
    }

    BGTileAtlas& BGTileAtlas::operator=(BGTileAtlas&& o) noexcept
    {
        if (this != &o)
        {
            dispose_();
            _name = std::move(o._name);
            _div = o._div;
            _soft_image = o._soft_image;
            _graphs_by_variant = std::move(o._graphs_by_variant);
            o._soft_image = -1;
        }
        return *this;
    }

    int BGTileAtlas::TilesPerVariant() const noexcept
    {
        if (_graphs_by_variant.empty()) return 0;
        return static_cast<int>(_graphs_by_variant.front().size());
    }

    void BGTileAtlas::DrawTile(int variant, int tile_index, int x, int y) const noexcept
    {
        if (variant < 0 || variant >= static_cast<int>(_graphs_by_variant.size())) return;
        const auto& tiles = _graphs_by_variant[static_cast<std::size_t>(variant)];

        if (tile_index < 0 || tile_index >= static_cast<int>(tiles.size())) return;
        const int h = tiles[static_cast<std::size_t>(tile_index)];

        if (h != -1)
        {
            ::DxLib::DrawGraph(x, y, h, FALSE);
        }
    }

    int BGTileAtlas::CreateTilePaletteVariant(int tile_index, std::span<const BGPaletteColorMapping> mappings)
    {
        using NESPalette = foundation::NES::NESPalette;

        if (_soft_image == -1)
        {
            return -1;
        }

        if (tile_index < 0 || tile_index >= TilesPerVariant())
        {
            return -1;
        }

        std::array<RGBA8, 256> base_palette{};

        for (int i = 0; i < 256; ++i)
        {
            int r = 0;
            int g = 0;
            int b = 0;
            int a = 255;

            if (::DxLib::GetPaletteSoftImage(
                _soft_image,
                i,
                &r,
                &g,
                &b,
                &a) != 0)
            {
                return -1;
            }

            base_palette[static_cast<std::size_t>(i)] =
                RGBA8{
                    static_cast<unsigned char>(r),
                    static_cast<unsigned char>(g),
                    static_cast<unsigned char>(b),
                    static_cast<unsigned char>(a)
            };
        }

        auto work_palette = base_palette;

        for (const BGPaletteColorMapping& mapping : mappings)
        {
            const auto& nes_color =
                NESPalette::GetColor(mapping.nesPaletteIndex);

            RGBA8& color =
                work_palette[
                    static_cast<std::size_t>(
                        mapping.pngPaletteIndex)];

            color.r =
                static_cast<unsigned char>(nes_color.red);

            color.g =
                static_cast<unsigned char>(nes_color.green);

            color.b =
                static_cast<unsigned char>(nes_color.blue);
        }

        for (int i = 0; i < 256; ++i)
        {
            const RGBA8& color =
                work_palette[static_cast<std::size_t>(i)];

            ::DxLib::SetPaletteSoftImage(
                _soft_image,
                i,
                color.r,
                color.g,
                color.b,
                color.a);
        }

        const int tx = tile_index % _div.tiles_x;
        const int ty = tile_index / _div.tiles_x;

        const int sx = tx * _div.tile_w;
        const int sy = ty * _div.tile_h;

        const int graph =
            ::DxLib::CreateGraphFromRectSoftImage(
                _soft_image,
                sx,
                sy,
                _div.tile_w,
                _div.tile_h);

        // Restore original PNG palette.
        for (int i = 0; i < 256; ++i)
        {
            const RGBA8& color =
                base_palette[static_cast<std::size_t>(i)];

            ::DxLib::SetPaletteSoftImage(
                _soft_image,
                i,
                color.r,
                color.g,
                color.b,
                color.a);
        }

        if (graph == -1)
        {
            return -1;
        }

        auto& variants =
            _tile_palette_variants[tile_index];

        variants.emplace_back(graph);

        return static_cast<int>(variants.size() - 1);
    }

    void BGTileAtlas::DrawTilePaletteVariant(int tile_index, int palette_variant, int x, int y) const noexcept
    {
        const auto it =
            _tile_palette_variants.find(tile_index);

        if (it == _tile_palette_variants.end())
        {
            DrawTile(0, tile_index, x, y);
            return;
        }

        const auto& variants = it->second;

        if (palette_variant < 0 ||
            palette_variant >=
            static_cast<int>(variants.size()))
        {
            DrawTile(0, tile_index, x, y);
            return;
        }

        const int graph =
            variants[
                static_cast<std::size_t>(
                    palette_variant)];

        if (graph != -1)
        {
            ::DxLib::DrawGraph(
                x,
                y,
                graph,
                FALSE);
        }
    }

    void BGTileAtlas::dispose_() noexcept
    {
        for (auto& [tile_index, variants] : _tile_palette_variants)
        {
            (void)tile_index;

            for (int handle : variants)
            {
                if (handle != -1)
                {
                    ::DxLib::DeleteGraph(handle);
                }
            }

            variants.clear();
        }
        _tile_palette_variants.clear();

        for (auto& tiles : _graphs_by_variant)
        {
            for (int h : tiles)
            {
                if (h != -1)
                {
                    ::DxLib::DeleteGraph(h);
                }
            }
            tiles.clear();
        }
        _graphs_by_variant.clear();

        if (_soft_image != -1)
        {
            ::DxLib::DeleteSoftImage(_soft_image);
            _soft_image = -1;
        }
    }
}