#include "pch.h"

#include "MapPageCache.h"

#include <cstdint>
#include <optional>
#include "AddressScraper.h"
#include "cstring"

namespace
{
    using conf = mm2hack::config::SystemConfig;
    constexpr std::size_t kPageSize = conf::kMapBinaryUnitPageSize;
    constexpr std::size_t kPayloadOff = conf::kMapBinaryHeaderSize;
}


namespace mm2hack::apps::resources::bg
{
    static_assert(PageTiles::kSize == 240, "16x15=240 である必要があります");

    void MapPageCache::BuildAround(const std::size_t currentPageIndex)
    {
        // 現在＋8近傍（最大9枚）を用意。無効な近傍はスキップ。
        // すでに _cache にある場合は上書き（bin の更新にも対応）
        auto ensure = [&](std::optional<std::size_t> idxOpt)
            {
                if (!idxOpt) return;
                const auto idx = *idxOpt;
                _cache[idx] = readTiles(idx);
            };

        // 現在
        _cache[currentPageIndex] = readTiles(currentPageIndex);

        // 4 近傍
        const auto r = Right(currentPageIndex);
        const auto l = Left(currentPageIndex);
        const auto u = Up(currentPageIndex);
        const auto d = Down(currentPageIndex);

        ensure(r); ensure(l); ensure(u); ensure(d);

        // 斜め：横→縦（いずれも存在する場合のみ）
        if (r && d) ensure(RightDown(currentPageIndex));
        if (l && d) ensure(LeftDown(currentPageIndex));
        if (r && u) ensure(RightUp(currentPageIndex));
        if (l && u) ensure(LeftUp(currentPageIndex));
    }

    std::uint8_t MapPageCache::Tile(const std::size_t pageIndex, const int tx, const int ty) const
    {
        if (tx < 0 || ty < 0 || tx >= PageTiles::kW || ty >= PageTiles::kH) return 0;

        auto it = _cache.find(pageIndex);
        if (it == _cache.end())
        {
            // 遅延ロード（BuildAround を呼び忘れても安全に参照可能）
            auto inserted = _cache.emplace(pageIndex, readTiles(pageIndex));
            it = inserted.first;
        }

        const auto& cells = it->second.cells;
        return cells[static_cast<std::size_t>(ty) * PageTiles::kW + static_cast<std::size_t>(tx)];
    }

    // 近傍解決：AddressScraper は pageIndex→隣室の roomNo を返す前提。
    // そこから pageIndex を再解決。
    std::optional<std::size_t> MapPageCache::Right(const std::size_t page) const
    {
        const int16_t roomNo = _s.getRightRoom(static_cast<int>(page));
        const int16_t idx = _s.getPageIndex(static_cast<std::size_t>(roomNo));
        return toOptIndex(idx);
    }

    std::optional<std::size_t> MapPageCache::Left(const std::size_t page) const
    {
        const int16_t roomNo = _s.getLeftRoom(static_cast<int>(page));
        const int16_t idx = _s.getPageIndex(static_cast<std::size_t>(roomNo));
        return toOptIndex(idx);
    }

    std::optional<std::size_t> MapPageCache::Up(const std::size_t page) const
    {
        const int16_t roomNo = _s.getOverRoom(static_cast<int>(page));
        const int16_t idx = _s.getPageIndex(static_cast<std::size_t>(roomNo));
        return toOptIndex(idx);
    }

    std::optional<std::size_t> MapPageCache::Down(const std::size_t page) const
    {
        const int16_t roomNo = _s.getUnderRoom(static_cast<int>(page));
        const int16_t idx = _s.getPageIndex(static_cast<std::size_t>(roomNo));
        return toOptIndex(idx);
    }

    std::optional<std::size_t> MapPageCache::RightDown(const std::size_t page) const
    {
        if (auto r = Right(page))
        {
            const int16_t roomNo = _s.getUnderRoom(static_cast<int>(*r));
            const int16_t idx = _s.getPageIndex(static_cast<std::size_t>(roomNo));
            return toOptIndex(idx);
        }
        return std::nullopt;
    }

    std::optional<std::size_t> MapPageCache::LeftDown(const std::size_t page) const
    {
        if (auto l = Left(page))
        {
            const int16_t roomNo = _s.getUnderRoom(static_cast<int>(*l));
            const int16_t idx = _s.getPageIndex(static_cast<std::size_t>(roomNo));
            return toOptIndex(idx);
        }
        return std::nullopt;
    }

    std::optional<std::size_t> MapPageCache::RightUp(const std::size_t page) const
    {
        if (auto r = Right(page))
        {
            const int16_t roomNo = _s.getOverRoom(static_cast<int>(*r));
            const int16_t idx = _s.getPageIndex(static_cast<std::size_t>(roomNo));
            return toOptIndex(idx);
        }
        return std::nullopt;
    }

    std::optional<std::size_t> MapPageCache::LeftUp(const std::size_t page) const
    {
        if (auto l = Left(page))
        {
            const int16_t roomNo = _s.getOverRoom(static_cast<int>(*l));
            const int16_t idx = _s.getPageIndex(static_cast<std::size_t>(roomNo));
            return toOptIndex(idx);
        }
        return std::nullopt;
    }

    PageTiles MapPageCache::readTiles(const std::size_t pageIndex) const
    {
        // AddressScraper の内部 bin 全体を参照（コピー不要）
        const auto& bin = _s.GetBin(); // ★ const ゲッター（末尾の最小パッチ参照）

        const std::size_t off = pageIndex * kPageSize + kPayloadOff;
        const std::size_t need = PageTiles::kSize; // 240
        PageTiles tiles{};

        if (off + need <= bin.size())
        {
            // 連続 240B を 16x15 順（左→右→下へ）でコピー
            std::memcpy(tiles.cells.data(), bin.data() + off, need);
        }
        else
        {
            // サイズ不足：安全のため 0 埋めのまま返す
        }
        return tiles;
    }
}