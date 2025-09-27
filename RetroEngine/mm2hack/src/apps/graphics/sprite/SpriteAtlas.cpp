#include "pch.h"

#include "SpriteAtlas.h"

#include <random>
#include <utility>
#include "apps/NES/NESPalette.h"

namespace mm2hack::apps::graphics::sprite
{
    SpriteAtlas::SpriteAtlas(std::wstring name,
                             DivSettings div,
                             int soft_image_handle,
                             std::vector<std::vector<int>> graphs_by_variant) noexcept
        : _name(std::move(name))
        , _div(div)
        , _soft_image(soft_image_handle)
        , _graphs_by_variant(std::move(graphs_by_variant))
    {
    }

    SpriteAtlas::~SpriteAtlas()
    {
        Dispose();
    }

    SpriteAtlas::SpriteAtlas(SpriteAtlas&& other) noexcept
        : _name(std::move(other._name))
        , _div(other._div)
        , _soft_image(other._soft_image)
        , _graphs_by_variant(std::move(other._graphs_by_variant))
    {
        other._soft_image = -1;
    }

    SpriteAtlas& SpriteAtlas::operator=(SpriteAtlas&& other) noexcept
    {
        if (this != &other)
        {
            Dispose();
            _name = std::move(other._name);
            _div = other._div;
            _soft_image = other._soft_image;
            _graphs_by_variant = std::move(other._graphs_by_variant);
            other._soft_image = -1;
        }
        return *this;
    }

    int SpriteAtlas::FramesPerVariant() const noexcept
    {
        if (_graphs_by_variant.empty())
        {
            return 0;
        }
        return static_cast<int>(_graphs_by_variant.front().size());
    }

    void SpriteAtlas::Draw(int variant, int frame, int x, int y) const noexcept
    {
        if (variant < 0 || variant >= static_cast<int>(_graphs_by_variant.size()))
        {
            return;
        }

        const auto& frames = _graphs_by_variant[static_cast<std::size_t>(variant)];
        if (frame < 0 || frame >= static_cast<int>(frames.size()))
        {
            return;
        }

        const int handle = frames[static_cast<std::size_t>(frame)];
        if (handle != -1)
        {
            ::DxLib::DrawGraph(x, y, handle, TRUE);
        }
    }

    bool SpriteAtlas::ReplacePaletteColorIndex(int variant, int targetPaletteIndex, int sourcePaletteIndex) noexcept
    {
        using NES::NESPalette;
        const auto& c = NESPalette::GetColor(targetPaletteIndex);
        return ReplacePaletteColorRGB(variant,
            static_cast<unsigned char>(c.red),
            static_cast<unsigned char>(c.green),
            static_cast<unsigned char>(c.blue),
            sourcePaletteIndex);
    }

    bool SpriteAtlas::ReplacePaletteColorRGB(int variant,
        unsigned char r, unsigned char g, unsigned char b,
        int sourcePaletteIndex) noexcept
    {
        if (_soft_image == -1) return false;

        // Replace the color in the soft image's palette.
        if (::DxLib::SetPaletteSoftImage(_soft_image, sourcePaletteIndex, r, g, b, 255) != 0)
        {
            return false;
        }
        // Rebuild the variant's graphs from the updated soft image.
        return RebuildVariantFromSoftImage_(variant);
    }

    bool SpriteAtlas::ApplyHSBToVariant(int variant, int hueAdd, int satAdd, int briAdd) noexcept
    {
        if (variant < 0 || variant >= static_cast<int>(_graphs_by_variant.size()))
            return false;

        auto& frames = _graphs_by_variant[static_cast<std::size_t>(variant)];
        if (frames.empty()) return false;

        for (int h : frames)
        {
            if (h != -1)
            {
                // Apply HSB filter to the graph
                ::DxLib::GraphFilter(h, DX_GRAPH_FILTER_HSB, 0, hueAdd, satAdd, briAdd);
            }
        }
        return true;
    }

    bool SpriteAtlas::ApplyRandomHueToVariant(int variant) noexcept
    {
        static thread_local std::mt19937 rng{ std::random_device{}() };
        std::uniform_int_distribution<int> dist(-128, 127);
        return ApplyHSBToVariant(variant, dist(rng), 0, 0);
    }

    bool SpriteAtlas::RebuildVariantFromSoftImage_(int variant) noexcept
    {
        if (_soft_image == -1 || variant < 0) return false;

        const int total = _div.tiles_x * _div.tiles_y;
        std::vector<int> new_frames(static_cast<std::size_t>(total), -1);

        const int res = ::DxLib::CreateDivGraphFromSoftImage(
            _soft_image, total, _div.tiles_x, _div.tiles_y, _div.tile_w, _div.tile_h, new_frames.data());
        if (res != 0) return false;

        if (variant >= static_cast<int>(_graphs_by_variant.size()))
        {
            _graphs_by_variant.resize(static_cast<std::size_t>(variant) + 1);
        }
        for (int h : _graphs_by_variant[static_cast<std::size_t>(variant)])
        {
            if (h != -1) ::DxLib::DeleteGraph(h);
        }
        _graphs_by_variant[static_cast<std::size_t>(variant)] = std::move(new_frames);
        return true;
    }

    void SpriteAtlas::Dispose() noexcept
    {
        for (auto& frames : _graphs_by_variant)
        {
            for (int h : frames)
            {
                if (h != -1)
                {
                    ::DxLib::DeleteGraph(h);
                }
            }
            frames.clear();
        }
        _graphs_by_variant.clear();

        if (_soft_image != -1)
        {
            ::DxLib::DeleteSoftImage(_soft_image);
            _soft_image = -1;
        }
    }
}