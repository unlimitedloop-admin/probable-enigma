//==============================================================================
// 
//  Project: mm2hack
//  BGTileManager.h
// 
//  BGTileManager class for managing background tile textures using the ITextureObject interface.
// 
//==============================================================================
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include "BGTileCatalog.h"

namespace mm2hack::apps::graphics::bg
{
    // Manages background tile textures and their properties
    class BGTileManager
    {
    public:
        using Id = BGTileCatalog::Id;

        BGTileManager() = default;
        ~BGTileManager() = default;

        // Load a BG tile group from a file, identified by its name
        Id LoadTileset(const std::wstring& name, std::wstring_view png_path, std::wstring_view json_path);
        // Remove a BG tile group by its Id or name
        void RemoveTilesetById(Id id);
        // Remove a BG tile group by its name
        void RemoveTilesetByName(const std::wstring& name);
        // Remove all BG tile groups
        void ClearTilesets();

        // Draw a tile from the specified tileset by its Id
        void DrawTileById(Id id, int tile_index, int x, int y) const noexcept;
        // Draw a tile from the specified tileset by its Id and variant
        void DrawTileVariantById(Id id, int variant, int tile_index, int x, int y) const noexcept;

        // Map data (simple, raw tile id grid). Width/Height are in tiles
        void SetMapSize(int width, int height);
        // Load map data from a binary file, with an optional offset
        void LoadMapBinary(std::wstring_view map_file, int offset = 0x10);
        // Set tile at (x,y) in the map
        void SetTile(int x, int y, std::uint8_t id);
        // Get tile at (x,y) in the map
        std::uint8_t GetTile(int x, int y) const;
        // Set tile attributes (per tile-id)
        void SetTileAttribute(std::uint8_t tile_id, std::uint8_t attr);
        // Get tile attribute for the specified tile-id
        std::uint8_t GetTileAttribute(int x, int y) const;

        // Draw map with the specified tileset name
        void DrawMapByName(const std::wstring& tileset_name, int tile_px_w, int tile_px_h, int offset_x = 0, int offset_y = 0) const;

        // Variant (global palette step) control
        inline void SetGlobalVariant(int v) noexcept { _global_variant = v; }
        [[nodiscard]] inline int GlobalVariant() const noexcept { return _global_variant; }

        // Events passthrough
        inline void SetEvents(BGTileCatalog::Events e) { _catalog.SetEvents(std::move(e)); }

        // Name->Id helpers
        [[nodiscard]] inline std::optional<Id> TryGetId(const std::wstring& name) const { return _catalog.TryGetId(name); }
        [[nodiscard]] inline bool Has(const std::wstring& name) const { return _catalog.Has(name); }

    private:
        BGTileCatalog _catalog{};   // manages tile groups
        int _global_variant{ 0 };   // global palette variant for drawing

        // Map state
        int _map_w{ 16 };
        int _map_h{ 15 };
        std::vector<std::uint8_t> _tile_map;     // size: _map_w * _map_h
        std::vector<std::uint8_t> _tile_attr;    // by tile-id
    };
}