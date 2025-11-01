#include "pch.h"

#include "BGRoomBank.h"

#include <cstdint>
#include <fstream>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string_view>
#include "exceptions/CoreException.h"

namespace
{
    inline uint8_t hi(uint8_t v) noexcept { return static_cast<uint8_t>((v >> 4) & 0x0F); }
    inline uint8_t lo(uint8_t v) noexcept { return static_cast<uint8_t>(v & 0x0F); }

    inline bool valid_bank(uint8_t v) noexcept { return v == 0x10 || v == 0x1A || v == 0x80; }
}


namespace mm2hack::apps::resources::bg
{
    // 安全にヘッダを解析する: バイトごとにフィールドを設定してパディング問題を避ける
    inline static BGRoomBank::Header read_hdr(const uint8_t* p)
    {
        BGRoomBank::Header h{};
        if (p == nullptr) return h;

        // ヘッダ領域は $00..$0F の 16 バイトで個別の役割を持つ
        h.magic0     = p[0];
        h.magic1     = p[1];
        h.page_hi    = p[2];
        h.bankMarker = p[3];
        h.room_id    = p[4];
        h.up_id      = p[5];
        h.down_id    = p[6];
        h.left_id    = p[7];
        h.right_id   = p[8];
        h.v_scroll   = p[9];
        h.h_scroll   = p[10];
        h.attr_type  = p[11];
        h.anim_type  = p[12];
        h.user0      = p[13];
        h.user1      = p[14];
        h.tail       = p[15];
        return h;
    }

    void BGRoomBank::Load(std::wstring_view bin_path)
    {
        const int kPageSize = static_cast<int>(config::SystemConfig::kMapBinaryUnitPageSize);

        std::ifstream ifs(std::wstring(bin_path), std::ios::binary);
        if (!ifs) THROW_EXCEPTION(L"BGRoomBank: cannot open file", kClassName);

        std::vector<uint8_t> bytes{ std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>() };
        if (bytes.empty() || (bytes.size() % kPageSize) != 0)
            throw std::runtime_error("BGRoomBank: file size is not multiple of " + std::to_string(kPageSize));

        _file = std::wstring(bin_path);
        _pages.clear();
        const size_t page_count = bytes.size() / kPageSize;
        _pages.reserve(page_count);

        for (size_t i = 0; i < page_count; ++i)
        {
            const size_t off = i * kPageSize;
            // ヘッダサイズ分だけ読み込んでもページを越えないことを明示的にチェック
            if (off + sizeof(BGRoomBank::Header) > bytes.size())
                THROW_EXCEPTION(L"BGRoomBank: unexpected file size while reading header", kClassName);

            const uint8_t* base = bytes.data() + off;

            const auto h = read_hdr(base);
            if (h.magic0 != 0x01 || h.magic1 != 0xFF || h.tail != 0xFF || !valid_bank(h.bankMarker))
                THROW_EXCEPTION(L"BGRoomBank: invalid header", kClassName);

            PageInfo pi{};
            pi.hdr = h;
            pi.file_offset_page = off;
            _pages.emplace_back(pi);
        }
    }

    std::optional<size_t> BGRoomBank::FindIndexByRoomId(uint8_t room_id) const noexcept
    {
        for (size_t i = 0; i < _pages.size(); ++i)
            if (_pages[i].hdr.room_id == room_id) return i;
        return std::nullopt;
    }

    std::optional<size_t> BGRoomBank::NeighborIndex(size_t index, Dir d) const noexcept
    {
        if (index >= _pages.size()) return std::nullopt;
        const auto& h = _pages[index].hdr;

        const uint8_t rid =
            (d == Dir::Up) ? h.up_id :
            (d == Dir::Down) ? h.down_id :
            (d == Dir::Left) ? h.left_id : h.right_id;

        if (rid == 0x00) return std::nullopt; // 0=隣接なしルール

        return FindIndexByRoomId(rid);
    }

    BGRoomBank::Scroll BGRoomBank::GetScroll(size_t index, Dir d) const noexcept
    {
        if (index >= _pages.size()) return Scroll::Other;
        const auto& h = _pages[index].hdr;

        const uint8_t v =
            (d == Dir::Up) ? hi(h.v_scroll) :
            (d == Dir::Down) ? lo(h.v_scroll) :
            (d == Dir::Left) ? hi(h.h_scroll) : lo(h.h_scroll);

        return static_cast<Scroll>(v);
    }
}