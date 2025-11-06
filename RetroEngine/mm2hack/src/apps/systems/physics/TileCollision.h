//==============================================================================
// 
//  Project: mm2hack
//  TileCollision.h
// 
//  Tile collision detection and response.
// 
//==============================================================================
#pragma once

#include <cstdint>
#include <string>

namespace mm2hack::apps::rendering::bg
{
    class BGTileManager;
}

namespace mm2hack::apps::systems::physics
{
    // BG field tile attributes definition
    enum class Attr : uint8_t
    {
        Space = 0,   // Null collision
        Solid = 1,   // Wall/Floor
        Ladder = 2,  // Up/Down ladder
        OneWay = 3,  // One-way floor (passable from below, non-collidable from above)
        Spike = 4    // Spike (damage/miss)
    };

    struct AABB
    {
        float x, y, w, h;
    };

    struct ResolveResult
    {
        bool on_ground{ false };
        bool on_ladder{ false };
        bool touched_spike{ false };
    };

    // Tile grid collision resolution (axis separation for X->Y)
    // Pixel -> Tile conversion depends on tile_px. Offsets will be used for future scrolling support
    class TileCollisionResolver
    {
        using BGTileManager = apps::rendering::bg::BGTileManager;

    public:
        TileCollisionResolver(const BGTileManager& mgr,
            int map_w_tiles, int map_h_tiles,
            int tile_px)
            : _mgr(mgr), _mw(map_w_tiles), _mh(map_h_tiles), _ts(tile_px)
        {
        }

        // Resolve collision for the given AABB with velocity (vx, vy).
        ResolveResult Resolve(AABB& box, float& vx, float& vy,
            bool allow_one_way_pass_up = true,
            float ofs_x_px = 0.0f, float ofs_y_px = 0.0f) const;

    private:
        [[nodiscard]] uint8_t getAttrByPx_(int px, int py, float ofs_x_px, float ofs_y_px) const noexcept;  // Get tile attribute by pixel coordinates with offsets
        [[nodiscard]] bool isSolid_(int tx, int ty) const noexcept;     // Is tile solid
        [[nodiscard]] bool isOneWay_(int tx, int ty) const noexcept;    // Is tile one-way
        [[nodiscard]] bool isLadder_(int tx, int ty) const noexcept;    // Is tile ladder
        [[nodiscard]] bool isSpike_(int tx, int ty) const noexcept;     // Is tile spike

    private:
        const std::wstring kClassName = L"TileCollisionResolver";

        const BGTileManager& _mgr;
        int _mw, _mh, _ts;
    };
}