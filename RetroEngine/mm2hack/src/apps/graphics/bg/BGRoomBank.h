//==============================================================================
// 
//  Project: mm2hack
//  BGRoomBank.h
// 
//  Background room bank management.
// 
//==============================================================================
#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mm2hack::apps::graphics::bg
{
    // bin = 0x100 バイト × N ページ
    // 0x00..0x0F: ヘッダ / 0x10..0xFF: 16x15=240 バイトのタイルID
    class BGRoomBank
    {
    public:
        enum class Dir : uint8_t { Up, Down, Left, Right };
        enum class Scroll : uint8_t
        {
            None = 0, Free = 1, PerPage = 2, NoScrollEdge = 3, NoScroll = 4, Dynamic = 5, AxisFix = 6,
            Loop = 7, Auto = 8, AutoByObj = 9, Large = 10, Raster3D = 11, RasterFlag = 12,
            MultiBG = 13, Undef14 = 14, Other = 15
        };

        struct Header
        {
            // $00..$0F
            uint8_t magic0;      // $00 : 0x01 固定
            uint8_t magic1;      // $01 : 0xFF 固定
            uint8_t page_hi;     // $02 : ページ上位
            uint8_t bankMarker;  // $03 : 0x10 / 0x1A / 0x80
            uint8_t room_id;     // $04 : 部屋ID 0x00..0xFF

            uint8_t up_id;       // $05
            uint8_t down_id;     // $06
            uint8_t left_id;     // $07
            uint8_t right_id;    // $08

            uint8_t v_scroll;    // $09 : 上位=Up, 下位=Down
            uint8_t h_scroll;    // $0A : 上位=Left, 下位=Right

            uint8_t attr_type;   // $0B
            uint8_t anim_type;   // $0C
            uint8_t user0;       // $0D
            uint8_t user1;       // $0E
            uint8_t tail;        // $0F : 0xFF 固定
        };

        // 1ページのメタ情報
        struct PageInfo
        {
            Header hdr{};
            size_t file_offset_page{ 0 }; // ファイル先頭からのページ頭 (0x100 境界)
        };

        BGRoomBank() = default;

        // ファイル全体をスキャンしてヘッダ列を作る（ページ数だけメタ情報を持つ）
        void Load(std::wstring_view bin_path);

        [[nodiscard]] size_t PageCount() const noexcept { return _pages.size(); }
        [[nodiscard]] const PageInfo& GetByIndex(size_t index) const { return _pages.at(index); }

        // 部屋ID→ページインデックスを検索（見つからなければ nullopt）
        [[nodiscard]] std::optional<size_t> FindIndexByRoomId(uint8_t room_id) const noexcept;

        // 隣接先（ページの Header から部屋IDを取り、インデックスへ解決）
        [[nodiscard]] std::optional<size_t> NeighborIndex(size_t index, Dir d) const noexcept;

        // スクロール種別
        [[nodiscard]] Scroll GetScroll(size_t index, Dir d) const noexcept;

        // 属性タイプ（$0B）
        [[nodiscard]] uint8_t AttrType(size_t index) const noexcept { return _pages.at(index).hdr.attr_type; }

        // 指定ページの 0x10 オフセット（BGTileManager::LoadMapBinary にそのまま渡せる）
        [[nodiscard]] int PayloadOffset(size_t index) const noexcept { return static_cast<int>(_pages.at(index).file_offset_page + 0x10); }

        // マップファイルパス
        [[nodiscard]] const std::wstring& FilePath() const noexcept { return _file; }

    private:
        const std::wstring kClassName{ L"BGRoomBank" };

        std::wstring _file{};
        std::vector<PageInfo> _pages;
    };
}