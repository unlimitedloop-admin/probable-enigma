#include "pch.h"

#include "SpriteCatalog.h"

#include <limits>
#include <nlohmann/json.hpp>
#include <optional>
#include <utility>
#include "SpriteAtlas.h"

namespace
{
    // Utility: safe int multiply with basic bounds
    [[nodiscard]] int mul_safe(int a, int b)
    {
        long long v = static_cast<long long>(a) * static_cast<long long>(b);
        if (v > std::numeric_limits<int>::max())
        {
            return std::numeric_limits<int>::max();
        }
        if (v < 0)
        {
            return 0;
        }
        return static_cast<int>(v);
    }

    // TODO: Implement actual palette recolor on SoftImage.
    // Stub returns a duplicated handle or the same handle if not supported.
    int make_palette_variant_softimage(int base_soft, int /*variant_index*/, int /*fade_step*/)
    {
        // For now, reuse the base soft image to build identical variant graphs.
        // You can replace this by duplicating the soft image and mapping pixels
        // through a palette LUT, then return the new soft image handle.
        // e.g., DxLib::MakeSoftImage / GetPixelSoftImage / DrawPixelSoftImage.
        return base_soft;
    }
}

namespace mm2hack::apps::graphics::sprite
{
    SpriteCatalog::~SpriteCatalog()
    {
        // Ensure events for destruction (optional)
        for (Id id = 0; id < _atlases.size(); ++id)
        {
            if (_atlases[id])
            {
                if (_events.on_destroyed)
                {
                    _events.on_destroyed(id, _atlases[id]->Name());
                }
            }
        }
        // Unique_ptrs will free resources automatically via SpriteAtlas dtor
        _atlases.clear();
        _name_to_id.clear();
    }

    bool SpriteCatalog::Has(const std::wstring& name) const
    {
        return _name_to_id.find(name) != _name_to_id.end();
    }

    SpriteCatalog::Id SpriteCatalog::GetId(const std::wstring& name) const
    {
        return _name_to_id.at(name);
    }

    std::optional<SpriteCatalog::Id> SpriteCatalog::TryGetId(const std::wstring& name) const noexcept
    {
        if (auto it = _name_to_id.find(name); it != _name_to_id.end())
        {
            return it->second;
        }
        return std::nullopt;
    }

    const SpriteAtlas& SpriteCatalog::GetAtlas(Id id) const noexcept
    {
        return *(_atlases[static_cast<std::size_t>(id)]);
    }

    SpriteAtlas& SpriteCatalog::GetAtlas(Id id) noexcept
    {
        return *(_atlases[static_cast<std::size_t>(id)]);
    }

    bool SpriteCatalog::IsValid(Id id) const noexcept
    {
        return id < _atlases.size() && static_cast<bool>(_atlases[id]);
    }

    SpriteCatalog::Id SpriteCatalog::Load(const std::wstring& name, const std::wstring& png_path, const std::wstring& json_path)
    {
        auto it = _name_to_id.find(name);
        if (it != _name_to_id.end())
        {
            return it->second; // already loaded
        }

        auto atlas = BuildAtlas_(name, png_path, json_path);
        const Id id = NextId_();
        if (id == _atlases.size())
        {
            _atlases.emplace_back(std::move(atlas));
        }
        else
        {
            _atlases[static_cast<std::size_t>(id)] = std::move(atlas);
        }
        _name_to_id.emplace(name, id);

        if (_events.on_created)
        {
            _events.on_created(id, name);
        }
        return id;
    }

    void SpriteCatalog::Remove(Id id)
    {
        if (id >= _atlases.size() || !_atlases[id])
        {
            return;
        }
        if (_events.on_destroyed)
        {
            _events.on_destroyed(id, _atlases[id]->Name());
        }
        // erase from name index
        for (auto it = _name_to_id.begin(); it != _name_to_id.end(); ++it)
        {
            if (it->second == id)
            {
                _name_to_id.erase(it);
                break;
            }
        }
        _atlases[id].reset(); // dtor frees DxLib resources
    }

    void SpriteCatalog::Clear()
    {
        for (Id id = 0; id < _atlases.size(); ++id)
        {
            if (_atlases[id])
            {
                if (_events.on_destroyed)
                {
                    _events.on_destroyed(id, _atlases[id]->Name());
                }
                _atlases[id].reset();
            }
        }
        _name_to_id.clear();
    }

    SpriteCatalog::Id SpriteCatalog::NextId_() const noexcept
    {
        return static_cast<Id>(_atlases.size());
    }

    std::unique_ptr<SpriteAtlas> SpriteCatalog::BuildAtlas_(const std::wstring& name,
                                                            const std::wstring& png_path,
                                                            const std::wstring& json_path)
    {
        // Load JSON meta.
        SpriteAtlas::DivSettings div{};
        SpriteAtlas::PaletteConfig pal{};

        if (!json_path.empty())
        {
            std::ifstream ifs(json_path);
            if (ifs)
            {
                nlohmann::json j;
                ifs >> j;
                if (j.is_null())
                {
                    THROW_EXCEPTION(L"JSON parse failed", L"SpriteCatalog");
                }
                else if (j.contains("Loader"))
                {
                    auto& loader = j["Loader"];
                    div.tile_w = loader.value("tileWidth", div.tile_w);
                    div.tile_h = loader.value("tileHeight", div.tile_h);
                    div.tiles_x = loader.value("tilesX", div.tiles_x);
                    div.tiles_y = loader.value("tilesY", div.tiles_y);
                    pal.variant_count = loader.value("paletteVariants", pal.variant_count);  // e.g.: 4
                    pal.nes_fade_step = loader.value("nesFadeStep", pal.nes_fade_step);      // e.g.: 16
                }
            }
        }

        // Load SoftImage (source PNG)
        const int soft = ::DxLib::LoadSoftImage(png_path.c_str());
        if (soft == -1)
        {
            THROW_EXCEPTION(L"LoadSoftImage failed", L"SpriteCatalog");
        }

        const int frame_count = mul_safe(div.tiles_x, div.tiles_y);
        std::vector<std::vector<int>> graphs_by_variant;
        graphs_by_variant.reserve(static_cast<std::size_t>(pal.variant_count));

        for (int v = 0; v < pal.variant_count; ++v)
        {
            const int src_soft = make_palette_variant_softimage(soft, v, pal.nes_fade_step);

            std::vector<int> frames(static_cast<std::size_t>(frame_count), -1);
            // NOTE: DxLib has CreateDivGraphFromSoftImage. If not available in your version, switch to CreateGraphFromSoftImage per tile.
            const int created = ::DxLib::CreateDivGraphFromSoftImage(
                src_soft,
                div.tiles_x * div.tiles_y,
                div.tiles_x,
                div.tiles_y,
                div.tile_w,
                div.tile_h,
                frames.data());

            if (created == -1)
            {
                // Fallback per tile creation (safer across versions)
                for (int ty = 0; ty < div.tiles_y; ++ty)
                {
                    for (int tx = 0; tx < div.tiles_x; ++tx)
                    {
                        const int idx = ty * div.tiles_x + tx;
                        int gh = -1;
                        // Derive by rectangle copy into a new graph
                        // 1) Make empty graph
                        gh = ::DxLib::MakeGraph(div.tile_w, div.tile_h, TRUE);
                        if (gh != -1)
                        {
                            // 2) Blit from soft image into graph
                            //    (Since DxLib lacks direct Soft->Graph blit by rect in some versions,
                            //     you may need intermediate RGBA buffer or DrawExtendGraph to a temp target.)
                            // TODO: Implement a robust rect copy path if needed.
                        }
                        frames[static_cast<std::size_t>(idx)] = gh;
                    }
                }
            }

            graphs_by_variant.emplace_back(std::move(frames));
        }

        // Keep or release the base soft image depending on future palette needs
        // Here we keep it (pass soft) so you can rebuild variants later if desired.
        return std::make_unique<SpriteAtlas>(name, div, soft, std::move(graphs_by_variant));
    }
}