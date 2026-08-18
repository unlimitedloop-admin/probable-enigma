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
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include "apps/systems/physics/TileAttribute.h"
#include "BGTileAnimator.h"
#include "BGTileCatalog.h"
#include "BGTilePalette.h"
#include "config/SystemConfig.h"

namespace mm2hack::apps::rendering::bg
{
    using systems::physics::TileAttribute;

    // Manages background tile textures and their properties
    class BGTileManager
    {
    public:
        using Id = BGTileCatalog::Id;
        using Byte = std::uint8_t;

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
        // Extract map binary data from a file, with an optional offset
        std::vector<std::uint8_t> ExtractMapBinary(std::wstring_view map_file) const;
        // Set tile attributes (per tile-id)
        void SetTileAttribute(std::uint8_t tile_id, TileAttribute attr);
        // Get tile attribute for the specified tile-id
        TileAttribute GetTileAttribute(int x, int y) const;
        TileAttribute GetTileAttribute(uint8_t id) const;

        // Creates a local palette variant for the specified tile.
        [[nodiscard]] int CreateTilePaletteVariantById(Id tileset_id, int tile_index, std::span<const BGPaletteColorMapping> mappings);
        // Creates a local palette variant for the specified tile by tileset name.
        [[nodiscard]] int CreateTilePaletteVariantByName(const std::wstring& tileset_name, int tile_index, std::span<const BGPaletteColorMapping> mappings);
        // Set BG tile palette animation definitions
        void SetTilePaletteAnimations(std::span<const BGPaletteAnimation> animations) noexcept;

        // Draw map with the specified tileset name
        void DrawMapByName(const std::wstring& tileset_name, int tile_px_w, int tile_px_h, int offset_x = 0, int offset_y = 0) const;
        // Draw map with the specified tileset Id
        void DrawMapById(Id tileset_id, int tile_px_w, int tile_px_h, int offset_x = 0, int offset_y = 0) const;

        // Events passthrough
        inline void SetEvents(BGTileCatalog::Events e) { _catalog.SetEvents(std::move(e)); }

        // Name->Id helpers
        [[nodiscard]] inline std::optional<Id> TryGetId(const std::wstring& name) const { return _catalog.TryGetId(name); }
        [[nodiscard]] inline bool Has(const std::wstring& name) const { return _catalog.Has(name); }

        // Variant (global palette step) control
        inline void SetGlobalVariant(int v) noexcept { _global_variant = v; }
        [[nodiscard]] inline int GlobalVariant() const noexcept { return _global_variant; }
        // Variant info
        [[nodiscard]] inline int MaxVariant() const noexcept { return _catalog.MaxVariantAcross(); }
        [[nodiscard]] int VariantCountByName(const std::wstring& tileset_name) const;
        [[nodiscard]] int VariantCountById(Id id) const;

        // Set global variant, clamped to valid range
        void SetGlobalVariantClamped(int v) noexcept;

        // Map info
        [[nodiscard]] inline int MapWidth() const noexcept { return _map_w; }
        [[nodiscard]] inline int MapHeight() const noexcept { return _map_h; }
        [[nodiscard]] inline int TileSize() const noexcept { return 16; }       // fixed 16x16 pixels

        // Set BG tile animation definitions
        void SetTileAnimations(std::span<const BGTileAnimation> animations) noexcept;
        // Advance BG tile animations by one frame
        void UpdateTileAnimations() noexcept;
        // Reset BG tile animations
        void ResetTileAnimations() noexcept;

    private:
        const std::wstring kClassName{ L"BGTileManager" };

        BGTileCatalog _catalog{};   // manages tile groups
        int _global_variant{ 0 };   // global palette variant for drawing

        // Map state
        int _map_w{ config::SystemConfig::kTileCountX };
        int _map_h{ config::SystemConfig::kTileCountY };
        std::vector<Byte> _tile_map;            // size: _map_w * _map_h
        std::vector<TileAttribute> _tile_attr;  // by tile-id
 
        BGTileAnimator _tile_animator{};        // manages tile animations
    };
}