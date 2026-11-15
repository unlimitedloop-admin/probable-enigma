#include "pch.h"

#include "BGTileAtlas.h"

#include <span>

#include "apps/foundation/NES/NESPalette.h"
#include "apps/rendering/common/DividedGraphLoader.h"
#include "BGTilePalette.h"

namespace mm2hack::apps::rendering::bg
{
    using common::get_palette_256;
    using common::make_fade_palette;
    using common::Palette256;
    using common::RGBA8;
    using common::set_palette_256;

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
        , _tile_palette_variants(std::move(o._tile_palette_variants))
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
            _tile_palette_variants = std::move(o._tile_palette_variants);
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

        Palette256 base_palette{};
        if (!get_palette_256(_soft_image, base_palette))
        {
            return -1;
        }

        Palette256 recolored_palette = base_palette;
        for (const BGPaletteColorMapping& mapping : mappings)
        {
            const auto& nes_color = NESPalette::GetColor(mapping.nesPaletteIndex);
            RGBA8& color = recolored_palette[static_cast<std::size_t>(mapping.pngPaletteIndex)];
            color.r = static_cast<unsigned char>(nes_color.red);
            color.g = static_cast<unsigned char>(nes_color.green);
            color.b = static_cast<unsigned char>(nes_color.blue);
        }

        const int sx = (tile_index % _div.tiles_x) * _div.tile_w;
        const int sy = (tile_index / _div.tiles_x) * _div.tile_h;

        // One graph per fade step, darkened exactly like the atlas's own
        // _graphs_by_variant (see load_divided_graph()), so DrawMapById() can
        // pick the step matching the current global fade variant.
        const int fade_count = VariantCount();
        std::vector<int> graphs_by_fade;
        graphs_by_fade.reserve(static_cast<std::size_t>(fade_count));

        Palette256 work_palette{};
        bool created_all = true;
        for (int fade = 0; fade < fade_count; ++fade)
        {
            make_fade_palette(recolored_palette, fade, fade_count, work_palette);
            set_palette_256(_soft_image, work_palette);

            const int graph = ::DxLib::CreateGraphFromRectSoftImage(
                _soft_image, sx, sy, _div.tile_w, _div.tile_h);
            if (graph == -1)
            {
                created_all = false;
                break;
            }
            graphs_by_fade.push_back(graph);
        }

        // Restore original PNG palette.
        set_palette_256(_soft_image, base_palette);

        if (!created_all || graphs_by_fade.empty())
        {
            for (int graph : graphs_by_fade)
            {
                ::DxLib::DeleteGraph(graph);
            }
            return -1;
        }

        auto& variants = _tile_palette_variants[tile_index];
        variants.emplace_back(std::move(graphs_by_fade));
        return static_cast<int>(variants.size() - 1);
    }

    void BGTileAtlas::DrawTilePaletteVariant(int tile_index, int palette_variant, int fade_variant, int x, int y) const noexcept
    {
        const auto it = _tile_palette_variants.find(tile_index);
        if (it == _tile_palette_variants.end())
        {
            DrawTile(fade_variant, tile_index, x, y);
            return;
        }

        const auto& variants = it->second;
        if (palette_variant < 0 || palette_variant >= static_cast<int>(variants.size()))
        {
            DrawTile(fade_variant, tile_index, x, y);
            return;
        }

        const auto& graphs_by_fade = variants[static_cast<std::size_t>(palette_variant)];
        if (fade_variant < 0 || fade_variant >= static_cast<int>(graphs_by_fade.size()))
        {
            return;
        }

        const int graph = graphs_by_fade[static_cast<std::size_t>(fade_variant)];
        if (graph != -1)
        {
            ::DxLib::DrawGraph(x, y, graph, FALSE);
        }
    }

    void BGTileAtlas::dispose_() noexcept
    {
        for (auto& [tile_index, variants] : _tile_palette_variants)
        {
            (void)tile_index;

            for (auto& graphs_by_fade : variants)
            {
                for (int handle : graphs_by_fade)
                {
                    if (handle != -1)
                    {
                        ::DxLib::DeleteGraph(handle);
                    }
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
