#include "pch.h"

#include "SpriteAtlas.h"

#include <utility>

namespace mm2hack::apps::graphics
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