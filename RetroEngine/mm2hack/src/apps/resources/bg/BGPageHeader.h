//==============================================================================
// 
//  Project: mm2hack
//  BGPageHeader.h
// 
//  Page header definition matching SSE (StageSmith Editor) BD-005 spec.
//  This supersedes the legacy BGRoomBank::Header layout.
// 
//  Source of truth: 
//    /_ゲーム開発_sse_BD-005_ファイル仕様書.md  (section 3.3)
//    /_ゲーム開発_sse_BD-006_ファイル仕様書_ステージ定義データ_スキーマ設計.md
// 
//==============================================================================
#pragma once

#include <cstdint>
#include <cstddef>

namespace mm2hack::apps::resources::bg
{
    //--------------------------------------------------------------------------
    // Header magic values (integrity check)
    //--------------------------------------------------------------------------
    inline constexpr std::uint8_t kPageHeaderMagicStart = 0xA5; // $00
    inline constexpr std::uint8_t kPageHeaderMagicEnd = 0x5A; // $0F

    //--------------------------------------------------------------------------
    // RoomScrollType
    // 
    // Per-direction scroll behavior, as stored in the page header
    // ($08 ScrollLeft / $09 ScrollRight / $0A ScrollUp / $0B ScrollDown).
    // Each direction byte holds exactly ONE of these values independently,
    // so e.g. "8-way free scroll" is simply Free on both an axis pair
    // (Left=Free, Right=Free, Up=Free, Down=Free) -- no separate enum value
    // is needed for that case.
    // 
    // NOTE: This is intentionally a DIFFERENT type from
    // mm2hack::apps::systems::scrolling::atomic::ScrollKind. That enum
    // describes runtime camera behavior; this one describes the static,
    // authored connection data from the editor. Mapping between the two
    // (if any) is a concern of the runtime scrolling system, not of this
    // header decode layer.
    //--------------------------------------------------------------------------
    enum class RoomScrollType : std::uint8_t
    {
        None         = 0x00, // No neighbor / no scroll
        Free         = 0x01, // Free scroll, centered on player
        PageEdge     = 0x02, // Page-unit scroll on screen-edge contact
        Auto         = 0x03, // Time-driven auto scroll
        ObjectFollow = 0x04, // Scroll centered on a non-player object (e.g. enemy)
        EventDriven  = 0x05, // Special behavior, fully driven by game-logic/.def events
        Loop         = 0x06, // Looping room (horizontal or vertical, per which axis field holds it)
    };

    [[nodiscard]] inline constexpr bool IsValidRoomScrollType(std::uint8_t v) noexcept
    {
        return v <= static_cast<std::uint8_t>(RoomScrollType::Loop);
    }

    //--------------------------------------------------------------------------
    // Page header flags ($0D)
    //--------------------------------------------------------------------------
    namespace PageFlagBits
    {
        inline constexpr std::uint8_t ContinuePoint   = 0x01; // bit0
        inline constexpr std::uint8_t NoScrollBack    = 0x02; // bit1
        inline constexpr std::uint8_t PostEffects     = 0x04; // bit2
        inline constexpr std::uint8_t Darkness        = 0x08; // bit3
        inline constexpr std::uint8_t Wind            = 0x10; // bit4
        inline constexpr std::uint8_t GravityModifier = 0x20; // bit5
        // bit6, bit7 are undefined / reserved
    }

    //--------------------------------------------------------------------------
    // BGPageHeader
    // 
    // Exact match to BD-005 section 3.3. 16 bytes, fixed layout.
    // Fields are deliberately declared in file order and read/written
    // byte-by-byte (see Parse/ToBytes below) rather than relying on
    // struct packing, to stay safe regardless of compiler/alignment.
    //--------------------------------------------------------------------------
    struct BGPageHeader
    {
        std::uint8_t magicStart{ kPageHeaderMagicStart };  // $00
        std::uint8_t roomId{ 0 };                          // $01
        std::uint8_t leftRoomId{ 0xFF };                   // $02 (0xFF = no neighbor)
        std::uint8_t rightRoomId{ 0xFF };                  // $03
        std::uint8_t upRoomId{ 0xFF };                     // $04
        std::uint8_t downRoomId{ 0xFF };                   // $05
        std::uint8_t frontRoomId{ 0xFF };                  // $06
        std::uint8_t backRoomId{ 0xFF };                   // $07

        std::uint8_t scrollLeft{ 0 };                      // $08 (RoomScrollType)
        std::uint8_t scrollRight{ 0 };                     // $09 (RoomScrollType)
        std::uint8_t scrollUp{ 0 };                        // $0A (RoomScrollType)
        std::uint8_t scrollDown{ 0 };                      // $0B (RoomScrollType)

        std::uint8_t z{ 0 };                               // $0C
        std::uint8_t flags{ 0 };                           // $0D (see PageFlagBits)
        std::uint8_t reserved{ 0 };                        // $0E
        std::uint8_t magicEnd{ kPageHeaderMagicEnd };      // $0F

        //---- typed accessors -------------------------------------------------

        [[nodiscard]] RoomScrollType ScrollLeftType()  const noexcept { return static_cast<RoomScrollType>(scrollLeft); }
        [[nodiscard]] RoomScrollType ScrollRightType() const noexcept { return static_cast<RoomScrollType>(scrollRight); }
        [[nodiscard]] RoomScrollType ScrollUpType()    const noexcept { return static_cast<RoomScrollType>(scrollUp); }
        [[nodiscard]] RoomScrollType ScrollDownType()  const noexcept { return static_cast<RoomScrollType>(scrollDown); }

        [[nodiscard]] bool HasLeftNeighbor()  const noexcept { return leftRoomId != 0xFF; }
        [[nodiscard]] bool HasRightNeighbor() const noexcept { return rightRoomId != 0xFF; }
        [[nodiscard]] bool HasUpNeighbor()    const noexcept { return upRoomId != 0xFF; }
        [[nodiscard]] bool HasDownNeighbor()  const noexcept { return downRoomId != 0xFF; }
        [[nodiscard]] bool HasFrontNeighbor() const noexcept { return frontRoomId != 0xFF; }
        [[nodiscard]] bool HasBackNeighbor()  const noexcept { return backRoomId != 0xFF; }

        [[nodiscard]] bool IsContinuePoint() const noexcept
        {
            return (flags & PageFlagBits::ContinuePoint) != 0;
        }

        [[nodiscard]] bool IsNoScrollBack() const noexcept
        {
            return (flags & PageFlagBits::NoScrollBack) != 0;
        }

        [[nodiscard]] bool HasPostEffects() const noexcept
        {
            return (flags & PageFlagBits::PostEffects) != 0;
        }

        [[nodiscard]] bool IsDarkness() const noexcept
        {
            return (flags & PageFlagBits::Darkness) != 0;
        }

        [[nodiscard]] bool IsWind() const noexcept
        {
            return (flags & PageFlagBits::Wind) != 0;
        }

        [[nodiscard]] bool HasGravityModifier() const noexcept
        {
            return (flags & PageFlagBits::GravityModifier) != 0;
        }

        [[nodiscard]] bool HasValidMagic() const noexcept
        {
            return magicStart == kPageHeaderMagicStart && magicEnd == kPageHeaderMagicEnd;
        }
    };

    inline constexpr std::size_t kBGPageHeaderSize = 16;

    //--------------------------------------------------------------------------
    // Parse: byte-by-byte decode from a 16-byte buffer.
    // 'p' must point to at least kBGPageHeaderSize readable bytes.
    // Returns a zero-initialized (all-0x00) header if p is null; callers
    // should check HasValidMagic() on the result before trusting it.
    //--------------------------------------------------------------------------
    [[nodiscard]] inline BGPageHeader ParseBGPageHeader(const std::uint8_t* p) noexcept
    {
        BGPageHeader h{};
        if (p == nullptr)
        {
            h = BGPageHeader{};
            h.magicStart = 0;
            h.magicEnd = 0;
            h.leftRoomId = h.rightRoomId = h.upRoomId = h.downRoomId = h.frontRoomId = h.backRoomId = 0;
            return h;
        }

        h.magicStart  = p[0x00];

        h.roomId      = p[0x01];
        h.leftRoomId  = p[0x02];
        h.rightRoomId = p[0x03];
        h.upRoomId    = p[0x04];
        h.downRoomId  = p[0x05];
        h.frontRoomId = p[0x06];
        h.backRoomId  = p[0x07];

        h.scrollLeft  = p[0x08];
        h.scrollRight = p[0x09];
        h.scrollUp    = p[0x0A];
        h.scrollDown  = p[0x0B];

        h.z           = p[0x0C];

        h.flags       = p[0x0D];

        h.reserved    = p[0x0E];

        h.magicEnd    = p[0x0F];

        return h;
    }

    //--------------------------------------------------------------------------
    // ToBytes: byte-by-byte encode into a 16-byte buffer.
    // 'out' must point to at least kBGPageHeaderSize writable bytes.
    // (Primarily useful for tests / tooling that need to round-trip a
    // header; the game itself is read-only against .bin.)
    //--------------------------------------------------------------------------
    inline void BGPageHeaderToBytes(const BGPageHeader& h, std::uint8_t* out) noexcept
    {
        if (out == nullptr) return;

        out[0x00] = h.magicStart;
        out[0x01] = h.roomId;
        out[0x02] = h.leftRoomId;
        out[0x03] = h.rightRoomId;
        out[0x04] = h.upRoomId;
        out[0x05] = h.downRoomId;
        out[0x06] = h.frontRoomId;
        out[0x07] = h.backRoomId;

        out[0x08] = h.scrollLeft;
        out[0x09] = h.scrollRight;
        out[0x0A] = h.scrollUp;
        out[0x0B] = h.scrollDown;

        out[0x0C] = h.z;
        out[0x0D] = h.flags;
        out[0x0E] = h.reserved;
        out[0x0F] = h.magicEnd;
    }
}