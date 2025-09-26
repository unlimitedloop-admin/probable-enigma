#include "pch.h"

#include "BGTileAtlas.h"

#include <utility>

namespace mm2hack::apps::graphics::bg
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
        Dispose();
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
            Dispose();
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
            ::DxLib::DrawGraph(x, y, h, TRUE);
        }
    }

    void BGTileAtlas::Dispose() noexcept
    {
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