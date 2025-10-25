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
#include "apps/graphics/bg/BGTileManager.h"

namespace mm2hack::apps::algorithm::physics
{
    // BGTileManager::SetTileAttribute で設定される想定の属性値
    enum class Attr : uint8_t
    {
        Space = 0,   // 空間
        Solid = 1,   // 壁・床（完全衝突）
        Ladder = 2,  // はしご
        OneWay = 3,  // すり抜け床（下から上は通過、上から下へは非衝突）
        Spike = 4    // トゲ（ダメージ／ミス）
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

    // タイルグリッドに対する衝突解決（X→Yの軸分離）
    // ピクセル→タイルの変換は tile_px 依存。オフセットは今後のスクロール対応で使用。
    class TileCollisionResolver
    {
    public:
        TileCollisionResolver(const apps::graphics::bg::BGTileManager& mgr,
            int map_w_tiles, int map_h_tiles,
            int tile_px)
            : _mgr(mgr), _mw(map_w_tiles), _mh(map_h_tiles), _ts(tile_px)
        {
        }

        // AABB を与えて衝突解決。vx,vy は参照で更新。
        ResolveResult Resolve(AABB& box, float& vx, float& vy,
            bool allow_one_way_pass_up = true,
            float ofs_x_px = 0.0f, float ofs_y_px = 0.0f) const;

    private:
        const apps::graphics::bg::BGTileManager& _mgr;
        int _mw, _mh, _ts;

        [[nodiscard]] uint8_t getAttrByPx(int px, int py,
            float ofs_x_px, float ofs_y_px) const noexcept;
        [[nodiscard]] bool isSolid(int tx, int ty) const noexcept;
        [[nodiscard]] bool isOneWay(int tx, int ty) const noexcept;
        [[nodiscard]] bool isLadder(int tx, int ty) const noexcept;
        [[nodiscard]] bool isSpike(int tx, int ty) const noexcept;
    };
}