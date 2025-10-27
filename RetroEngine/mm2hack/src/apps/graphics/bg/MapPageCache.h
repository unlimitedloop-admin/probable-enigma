//==============================================================================
// 
//  Project: mm2hack
//  MapPageCache.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include "AddressScraper.h"

namespace mm2hack::apps::graphics::bg
{
    // 1ページ=16x15=240セル
    struct PageTiles
    {
        static constexpr int kW = 16;
        static constexpr int kH = 15;
        static constexpr int kSize = kW * kH; // 240
        std::array<std::uint8_t, kSize> cells{};
    };

    class MapPageCache
    {
    public:
        explicit MapPageCache(AddressScraper& s) : _s(s)
        {
        }

        // 現在ページが変わったタイミングで呼ぶ。現在＋近傍をまとめて準備
        void BuildAround(std::size_t currentPageIndex);

        // （遅延ロード可）指定ページの (tx,ty) のタイルIDを返す。範囲外は 0。
        std::uint8_t Tile(std::size_t pageIndex, int tx, int ty) const;

        // 近傍解決（見つからなければ nullopt）
        std::optional<std::size_t> Right(std::size_t page) const;
        std::optional<std::size_t> Left(std::size_t page) const;
        std::optional<std::size_t> Up(std::size_t page) const;
        std::optional<std::size_t> Down(std::size_t page) const;

        std::optional<std::size_t> RightDown(std::size_t page) const;
        std::optional<std::size_t> LeftDown(std::size_t page) const;
        std::optional<std::size_t> RightUp(std::size_t page) const;
        std::optional<std::size_t> LeftUp(std::size_t page) const;

    private:
        AddressScraper& _s;
        // ページインデックス → タイル配列
        mutable std::unordered_map<std::size_t, PageTiles> _cache;

        // 240B を bin から読み取って PageTiles を作る
        PageTiles readTiles(std::size_t pageIndex) const;

        // roomNo (uint8) → pageIndex (>=0) のラッパ
        static std::optional<std::size_t> toOptIndex(int16_t idx)
        {
            return (idx >= 0) ? std::optional<std::size_t>(static_cast<std::size_t>(idx)) : std::nullopt;
        }
    };
}