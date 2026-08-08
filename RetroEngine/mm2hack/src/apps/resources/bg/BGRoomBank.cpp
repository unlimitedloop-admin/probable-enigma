#include "pch.h"

#include "BGRoomBank.h"

#include <iterator>
#include <stdexcept>
#include <string_view>

namespace
{
    // No-neighbor sentinel per BD-005 (was 0x00 under the old layout)
    constexpr uint8_t kNoNeighbor = 0xFF;
}


namespace mm2hack::apps::resources::bg
{
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
            // Header reading safety check
            if (off + kBGPageHeaderSize > bytes.size())
                THROW_EXCEPTION(L"BGRoomBank: unexpected file size while reading header", kClassName);

            const uint8_t* base = bytes.data() + off;

            const auto h = ParseBGPageHeader(base);
            if (!h.HasValidMagic())
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
            if (_pages[i].hdr.roomId == room_id) return i;
        return std::nullopt;
    }

    std::optional<size_t> BGRoomBank::NeighborIndex(size_t index, Dir d) const noexcept
    {
        if (index >= _pages.size()) return std::nullopt;
        const auto& h = _pages[index].hdr;

        const uint8_t rid =
            (d == Dir::Up) ? h.upRoomId :
            (d == Dir::Down) ? h.downRoomId :
            (d == Dir::Left) ? h.leftRoomId : h.rightRoomId;

        if (rid == kNoNeighbor) return std::nullopt;   // No neighbor

        return FindIndexByRoomId(rid);
    }

    BGRoomBank::Scroll BGRoomBank::GetScroll(size_t index, Dir d) const noexcept
    {
        if (index >= _pages.size()) return Scroll::None;
        const auto& h = _pages[index].hdr;

        const uint8_t v =
            (d == Dir::Up) ? h.scrollUp :
            (d == Dir::Down) ? h.scrollDown :
            (d == Dir::Left) ? h.scrollLeft : h.scrollRight;

        return static_cast<Scroll>(v);
    }
}