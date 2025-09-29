#include "pch.h"

#include "BGTileManager.h"

#include <cstdint>
#include <iterator>
#include <string_view>

namespace mm2hack::apps::graphics::bg
{
    BGTileManager::Id BGTileManager::LoadTileset(const std::wstring& name, std::wstring_view png_path, std::wstring_view json_path)
    {
        return _catalog.Load(name, png_path, json_path);
    }

    void BGTileManager::RemoveTilesetById(Id id)
    {
        _catalog.Remove(id);
    }

    void BGTileManager::RemoveTilesetByName(const std::wstring& name)
    {
        if (auto opt = _catalog.TryGetId(name))
        {
            _catalog.Remove(*opt);
        }
    }

    void BGTileManager::ClearTilesets()
    {
        _catalog.Clear();
    }

    void BGTileManager::DrawTileById(Id id, int tile_index, int x, int y) const noexcept
    {
        if (!_catalog.IsValid(id)) return;
        
        _catalog.GetAtlas(id).DrawTile(_global_variant, tile_index, x, y);
    }

    void BGTileManager::DrawTileVariantById(Id id, int variant, int tile_index, int x, int y) const noexcept
    {
        if (!_catalog.IsValid(id)) return;
        
        _catalog.GetAtlas(id).DrawTile(variant, tile_index, x, y);
    }

    void BGTileManager::SetMapSize(int width, int height)
    {
        _map_w = width;
        _map_h = height;
        
        _tile_map.assign(_map_w * _map_h, 0);
    }

    void BGTileManager::LoadMapBinary(std::wstring_view map_file, int offset)
    {
        std::ifstream file(std::wstring(map_file), std::ios::binary);
        if (!file)
        {
            THROW_EXCEPTION(L"Failed to open map file: " + std::wstring(map_file), L"BGTileManager");
        }
        file.unsetf(std::ios::skipws);

        std::vector<std::uint8_t> raw(std::istream_iterator<std::uint8_t>{file}, {});
        const int need = _map_w * _map_h;
        if ((int)raw.size() < offset + need)
        {
            THROW_EXCEPTION(L"Map file is too small: " + std::wstring(map_file), L"BGTileManager");
        }

        _tile_map.assign(raw.begin() + offset, raw.begin() + offset + need);
    }

    void BGTileManager::SetTile(int x, int y, std::uint8_t id)
    {
        if (x < 0 || y < 0 || x >= _map_w || y >= _map_h) return;
        _tile_map[y * _map_w + x] = id;
    }

    std::uint8_t BGTileManager::GetTile(int x, int y) const
    {
        if (x < 0 || y < 0 || x >= _map_w || y >= _map_h) return 0;
        return _tile_map[y * _map_w + x];
    }

    void BGTileManager::SetTileAttribute(std::uint8_t tile_id, std::uint8_t attr)
    {
        if (_tile_attr.size() <= tile_id)
        {
            _tile_attr.resize(tile_id + 1);
        }
        _tile_attr[tile_id] = attr;
    }
    std::uint8_t BGTileManager::GetTileAttribute(int x, int y) const
    {
        if (x < 0 || y < 0 || x >= _map_w || y >= _map_h)
        {
            return 0;
        }

        const std::uint8_t id = _tile_map[y * _map_w + x];
        if (id < _tile_attr.size())
        {
            return _tile_attr[id];
        }

        return 0;
    }

    void BGTileManager::DrawMapByName(const std::wstring& tileset_name, int tile_px_w, int tile_px_h, int offset_x, int offset_y) const
    {
        auto opt = _catalog.TryGetId(tileset_name);
        if (!opt)
        {
            return;
        }
        
        const Id id = *opt;
        const auto& atlas = _catalog.GetAtlas(id);
        for (int y = 0; y < _map_h; ++y)
        {
            for (int x = 0; x < _map_w; ++x)
            {
                const int idx = y * _map_w + x;
                const int tile_id = static_cast<int>(_tile_map[idx]);
                atlas.DrawTile(_global_variant, tile_id, x * tile_px_w + offset_x, y * tile_px_h + offset_y);
            }
        }
    }

    int BGTileManager::VariantCountByName(const std::wstring& tileset_name) const
    {
        if (auto id = _catalog.TryGetId(tileset_name)) return _catalog.GetAtlas(*id).VariantCount();
        return 0;
    }

    int BGTileManager::VariantCountById(Id id) const
    {
        if (!_catalog.IsValid(id)) return 0;
        return _catalog.GetAtlas(id).VariantCount();
    }

    void BGTileManager::SetGlobalVariantClamped(int v) noexcept
    {
        const int mv = MaxVariant();
        _global_variant = std::max(0, std::min(v, mv));
    }

}