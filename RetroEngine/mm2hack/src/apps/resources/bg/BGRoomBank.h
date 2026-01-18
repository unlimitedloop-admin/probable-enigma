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
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mm2hack::apps::resources::bg
{
    // bin = 0x100 byte x N pages
    // 0x00..0x0F: header / 0x10..0xFF: 16x15=240 byte tile IDs
    class BGRoomBank
    {
    public:
        enum class Dir : uint8_t { Up, Down, Left, Right };
        enum class Scroll : uint8_t
        {
            None         = 0x00,
            Free         = 0x01,
            PerPage      = 0x02,
            NoScrollEdge = 0x03,
            NoScroll     = 0x04,
            Dynamic      = 0x05,
            AxisFix      = 0x06,
            Loop         = 0x07,
            Auto         = 0x08,
            AutoByObj    = 0x09,
            Large        = 0x0A,
            Raster3D     = 0x0B,
            RasterFlag   = 0x0C,
            MultiBG      = 0x0D,
            Undef14      = 0x0E,
            Other        = 0x0F
        };

        struct Header
        {
            // $00..$0F
            uint8_t magic0;      // $00 : 0x01 fixed
            uint8_t magic1;      // $01 : 0xFF fixed
            uint8_t page_hi;     // $02 : page high
            uint8_t bankMarker;  // $03 : 0x10 / 0x1A / 0x80
            uint8_t room_id;     // $04 : room ID 0x00..0xFF

            uint8_t up_id;       // $05
            uint8_t down_id;     // $06
            uint8_t left_id;     // $07
            uint8_t right_id;    // $08

            uint8_t v_scroll;    // $09 : upper=Up, lower=Down
            uint8_t h_scroll;    // $0A : upper=Left, lower=Right

            uint8_t attr_type;   // $0B
            uint8_t anim_type;   // $0C
            uint8_t user0;       // $0D
            uint8_t user1;       // $0E
            uint8_t tail;        // $0F : 0xFF fixed
        };

        // The page info including header and file offset
        struct PageInfo
        {
            Header hdr{};
            size_t file_offset_page{ 0 }; // File offset from the start of the file (0x100 boundary)
        };

        BGRoomBank() = default;

        // Scan the entire file to create a header list (only meta information for the number of pages)
        void Load(std::wstring_view bin_path);

        [[nodiscard]] size_t PageCount() const noexcept { return _pages.size(); }
        [[nodiscard]] const PageInfo& GetByIndex(size_t index) const { return _pages.at(index); }
        // room ID -> page index search (returns nullopt if not found)
        [[nodiscard]] std::optional<size_t> FindIndexByRoomId(uint8_t room_id) const noexcept;
        // Neighbor resolution (takes room ID from page header and resolves to index)
        [[nodiscard]] std::optional<size_t> NeighborIndex(size_t index, Dir d) const noexcept;
        // Scroll type
        [[nodiscard]] Scroll GetScroll(size_t index, Dir d) const noexcept;
        // Attribute type ($0B)
        [[nodiscard]] uint8_t AttrType(size_t index) const noexcept { return _pages.at(index).hdr.attr_type; }
        // Offset of the specified page (can be passed directly to BGTileManager::LoadMapBinary)
        [[nodiscard]] int PayloadOffset(size_t index) const noexcept { return static_cast<int>(_pages.at(index).file_offset_page + 0x10); }
        // Map file path
        [[nodiscard]] const std::wstring& FilePath() const noexcept { return _file; }

    private:
        const std::wstring kClassName{ L"BGRoomBank" };

        std::wstring _file{};
        std::vector<PageInfo> _pages;
    };
}