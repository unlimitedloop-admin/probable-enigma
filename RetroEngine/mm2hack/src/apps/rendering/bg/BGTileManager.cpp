#include "pch.h"

#include "BGTileManager.h"

#include <iterator>
#include <span>
#include <string_view>
#include "BGTileAnimator.h"
#include "BGTilePalette.h"

namespace mm2hack::apps::systems::physics
{
    enum class TileAttribute : std::uint16_t;
}

namespace mm2hack::apps::rendering::bg
{
    BGTileManager::Id BGTileManager::LoadTileset(const std::wstring& name, std::wstring_view png_path, std::wstring_view json_path)
    {
        const std::wstring png = std::wstring(png_path);
        const std::wstring json = std::wstring(json_path);
        return _catalog.Load(name, png, json);
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
        // OPTIMIZE: Since this method is called every frame, please check the performance impact of any sections that perform disk I/O.
        std::ifstream file(std::wstring(map_file), std::ios::binary);
        if (!file)
        {
            THROW_EXCEPTION(L"Failed to open map file: " + std::wstring(map_file), kClassName);
        }
        file.unsetf(std::ios::skipws);

        std::vector<std::uint8_t> raw(std::istream_iterator<std::uint8_t>{file}, {});
        const int need = _map_w * _map_h;
        if ((int)raw.size() < offset + need)
        {
            THROW_EXCEPTION(L"Map file is too small: " + std::wstring(map_file), kClassName);
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

    std::vector<std::uint8_t> BGTileManager::ExtractMapBinary(std::wstring_view map_file) const
    {
        const std::wstring path = std::wstring(map_file);
        std::ifstream file(path, std::ios::binary);
        if (!file)
        {
            THROW_EXCEPTION(L"Failed to open map file: " + path, kClassName);
        }

        // Get seekg size
        file.seekg(0, std::ios::end);
        std::streamoff s = file.tellg();
        if (s <= 0)
        {
            // Empty file or failed to obtain
            return std::vector<std::uint8_t>();
        }
        file.seekg(0, std::ios::beg);

        // Allocate buffer and read all at once
        std::vector<std::uint8_t> raw(static_cast<std::size_t>(s));
        file.read(reinterpret_cast<char*>(raw.data()), static_cast<std::streamsize>(raw.size()));

        // If reading failed midway, resize to match the number of bytes read
        if (!file)
        {
            std::streamsize readBytes = file.gcount();
            if (readBytes <= 0)
            {
                return std::vector<std::uint8_t>();
            }
            raw.resize(static_cast<std::size_t>(readBytes));
        }

        return raw;
    }

    void BGTileManager::SetTileAttribute(std::uint8_t tile_id, TileAttribute attr)
    {
        if (_tile_attr.size() <= tile_id)
        {
            _tile_attr.resize(tile_id + 1);
        }
        _tile_attr[tile_id] = attr;
    }

    TileAttribute BGTileManager::GetTileAttribute(int x, int y) const
    {
        if (x < 0 || y < 0 || x >= _map_w || y >= _map_h) { return TileAttribute(); }

        const std::uint8_t id = _tile_map[y * _map_w + x];
        
        if (id < _tile_attr.size()) { return _tile_attr[id]; }
        
        return TileAttribute();
    }

    TileAttribute BGTileManager::GetTileAttribute(uint8_t id) const
    {
        return id < _tile_attr.size() ? _tile_attr[id] : TileAttribute();
    }

    int BGTileManager::CreateTilePaletteVariantById(Id tileset_id, int tile_index, std::span<const BGPaletteColorMapping> mappings)
    {
        if (!_catalog.IsValid(tileset_id))
        {
            return -1;
        }

        auto& atlas = _catalog.GetAtlas(tileset_id);
        return atlas.CreateTilePaletteVariant(tile_index, mappings);
    }

    int BGTileManager::CreateTilePaletteVariantByName(const std::wstring& tileset_name, int tile_index, std::span<const BGPaletteColorMapping> mappings)
    {
        const auto id = _catalog.TryGetId(tileset_name);

        if (!id.has_value())
        {
            return -1;
        }

        return CreateTilePaletteVariantById(*id, tile_index, mappings);
    }

    void BGTileManager::SetTilePaletteAnimations(std::span<const BGPaletteAnimation> animations) noexcept
    {
        _tile_animator.SetPaletteAnimations(animations);
    }

    void BGTileManager::DrawMapByName(const std::wstring& tileset_name, int tile_px_w, int tile_px_h, int offset_x, int offset_y) const
    {
        DrawMapById(_catalog.GetId(tileset_name), tile_px_w, tile_px_h, offset_x, offset_y);
    }

    void BGTileManager::DrawMapById(Id tileset_id, int tile_px_w, int tile_px_h, int offset_x, int offset_y) const
    {
        if (!_catalog.IsValid(tileset_id)) return;
        const auto& atlas = _catalog.GetAtlas(tileset_id);
        for (int y = 0; y < _map_h; ++y)
        {
            for (int x = 0; x < _map_w; ++x)
            {
                const int idx = y * _map_w + x;

                const std::uint8_t source_tile = _tile_map[idx];
                const std::uint8_t drawing_tile = _tile_animator.ResolveTile(source_tile);
                const int palette_variant = _tile_animator.ResolvePaletteVariant(source_tile);
                const int draw_x = x * tile_px_w + offset_x;
                const int draw_y = y * tile_px_h + offset_y;

                if (palette_variant >= 0)
                {
                    atlas.DrawTilePaletteVariant(static_cast<int>(drawing_tile), palette_variant, draw_x, draw_y);
                }
                else
                {
                    atlas.DrawTile(_global_variant, static_cast<int>(drawing_tile), draw_x, draw_y);
                }
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

    void BGTileManager::SetTileAnimations(
        std::span<const BGTileAnimation> animations) noexcept
    {
        _tile_animator.SetAnimations(animations);
    }

    void BGTileManager::UpdateTileAnimations() noexcept
    {
        _tile_animator.Update();
    }

    void BGTileManager::ResetTileAnimations() noexcept
    {
        _tile_animator.Reset();
    }
}