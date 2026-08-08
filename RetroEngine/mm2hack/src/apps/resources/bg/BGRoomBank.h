//==============================================================================
// 
//  Project: mm2hack
//  BGRoomBank.h
// 
//  Background room bank management.
// 
//  [MIGRATION NOTE]
//  Header layout now matches BD-005 (see BGPageHeader.h). The old
//  bankMarker/page_hi/attr_type/anim_type fields from the legacy layout
//  no longer exist in the same form -- see comments below for what
//  replaced each address.
// 
//==============================================================================
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include "BGPageHeader.h"

namespace mm2hack::apps::resources::bg
{
    // bin = 0x100 byte x N pages
    // 0x00..0x0F: header (BD-005 BGPageHeader) / 0x10..0xFF: 16x15=240 byte tile IDs
    //
    // HACK: This class overlaps with AddressScraper (both decode BGPageHeader
    // from the same .bin). They are NOT redundant in practice, though:
    //   - BGRoomBank: used once at scene entry (see e.g. DemoStage2::initializeResources_)
    //     to validate the .bin and resolve the starting RoomNo -> pageIndex via
    //     FindIndexByRoomId(). Owned as a long-lived singleton-scoped resource
    //     (ResourceManager::GetBGRoomBank()); its FilePath() is also reused by
    //     ActionStageRuntimeBuilder to re-extract the binary for AddressScraper.
    //   - AddressScraper: used every frame during gameplay (via MapPageCache /
    //     ScraperScrollRuleProvider) for neighbor + scroll-type queries.
    // If a 3rd call site needing BGPageHeader decode shows up, that's the
    // trigger (per the "3rd file" DRY rule) to actually merge these into one
    // decoder. Until then, leave both -- just keep BGPageHeader.h as the only
    // place the byte layout itself is defined.
    class BGRoomBank
    {
    public:
        enum class Dir : uint8_t { Up, Down, Left, Right };

        // Scroll type now matches BD-005 RoomScrollType (7 values, no nibble packing).
        // Kept as an alias here so existing call sites that spell out
        // BGRoomBank::Scroll don't all need renaming in one pass.
        using Scroll = RoomScrollType;

        // Header alias: this class now decodes the same layout as
        // AddressScraper via BGPageHeader. Kept as a member alias so
        // existing call sites (h.room_id, h.up_id, ...) don't all need
        // renaming in one pass -- BGPageHeader's field names differ
        // slightly (roomId, upRoomId, ...), so call sites DO need to be
        // updated to the new field names. See mapping table below.
        //
        //   OLD (BGRoomBank::Header) -> NEW (BGPageHeader)
        //   magic0/magic1            -> magicStart / (roomId now owns $01)
        //   page_hi, bankMarker      -> REMOVED (no BD-005 equivalent)
        //   room_id                  -> roomId
        //   up_id/down_id            -> upRoomId / downRoomId
        //   left_id/right_id         -> leftRoomId / rightRoomId
        //   v_scroll/h_scroll        -> REPLACED by 4 independent bytes:
        //                                scrollUp/scrollDown/scrollLeft/scrollRight
        //   attr_type ($0B)          -> now ScrollDown ($0B). See AttrType() TODO.
        //   anim_type ($0C)          -> now Z ($0C)
        //   user0/user1              -> flags ($0D) / reserved ($0E)
        //   tail                     -> magicEnd
        using Header = BGPageHeader;

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
        // Scroll type (BD-005 RoomScrollType, full byte per direction)
        [[nodiscard]] Scroll GetScroll(size_t index, Dir d) const noexcept;

        // Offset of the specified page (can be passed directly to BGTileManager::LoadMapBinary)
        [[nodiscard]] int PayloadOffset(size_t index) const noexcept { return static_cast<int>(_pages.at(index).file_offset_page + kBGPageHeaderSize); }
        // Map file path
        [[nodiscard]] const std::wstring& FilePath() const noexcept { return _file; }

    private:
        const std::wstring kClassName{ L"BGRoomBank" };

        std::wstring _file{};
        std::vector<PageInfo> _pages;
    };
}