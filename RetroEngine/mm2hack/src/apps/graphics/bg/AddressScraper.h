//==============================================================================
// 
//  Project: mm2hack
//  AddressScraper.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace mm2hack::apps::graphics::bg
{
    class AddressScraper
    {
    public:
        // bin: 0x100 * N ページ構造。各ページ [0x00..0x0F]=ヘッダ, [0x10..0xFF]=タイル240B
        explicit AddressScraper(std::vector<std::uint8_t> bin);

        // ユーティリティ
        [[nodiscard]] const std::vector<std::uint8_t>& binary() const noexcept { return _bin; }
        [[nodiscard]] std::size_t pageCount() const noexcept { return _bin.size() / 0x100; }

        // マップ：room_id(0x04) → pageIndex
        [[nodiscard]] int16_t getPageIndex(std::size_t roomNo) const;  // 見つからなければ -1
        [[nodiscard]] int16_t getcurrentRoomNo(std::size_t pageIndex) const;

        // 隣室の room_id を返す（無ければ -1）。仕様：$05=上, $06=下, $07=左, $08=右
        [[nodiscard]] int16_t getOverRoom(std::size_t pageIndex) const;
        [[nodiscard]] int16_t getUnderRoom(std::size_t pageIndex) const;
        [[nodiscard]] int16_t getLeftRoom(std::size_t pageIndex) const;
        [[nodiscard]] int16_t getRightRoom(std::size_t pageIndex) const;

        // スクロール種別（ニブル）：$09 上位=Up, 下位=Down / $0A 上位=Left, 下位=Right
        [[nodiscard]] int16_t getOverScrollType(std::size_t pageIndex) const;
        [[nodiscard]] int16_t getUnderScrollType(std::size_t pageIndex) const;
        [[nodiscard]] int16_t getLeftScrollType(std::size_t pageIndex) const;
        [[nodiscard]] int16_t getRightScrollType(std::size_t pageIndex) const;

        // スクロール可能か？（部屋IDが存在し、ニブルが {1,9,0x0A} のとき true）
        [[nodiscard]] bool isPossibleGoOver(std::size_t pageIndex) const;
        [[nodiscard]] bool isPossibleGoUnder(std::size_t pageIndex) const;
        [[nodiscard]] bool isPossibleGoLeft(std::size_t pageIndex) const;
        [[nodiscard]] bool isPossibleGoRight(std::size_t pageIndex) const;

        // 240B のタイル領域 [0x10..0xFF] の先頭ポインタ（境界内なら）
        [[nodiscard]] const std::uint8_t* payloadPtr(std::size_t pageIndex) const noexcept;

        std::vector<std::uint8_t> GetBin() const { return _bin; }

    private:
        void buildPageIndexMap_();
        [[nodiscard]] bool inRange_(std::size_t pageIndex) const noexcept;
        [[nodiscard]] std::uint8_t H_(std::size_t pageIndex, std::size_t off) const noexcept;   // ヘッダ1B取得
        [[nodiscard]] static bool isScrollableNibble_(std::uint8_t v) noexcept
        {
            return v == 0x01 || v == 0x09 || v == 0x0A;
        }

    private:
        std::vector<std::uint8_t> _bin;
        std::unordered_map<std::size_t, int16_t> _roomToPage; // room_id -> pageIndex
    };
}