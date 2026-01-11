#include "pch.h"

#include "SpriteCatalog.h"

#include <cmath>
#include <limits>
#include <nlohmann/json.hpp>
#include "SpriteAtlas.h"

namespace
{
    [[nodiscard]] int mul_safe(int a, int b)
    {
        long long v = 1LL * a * b;
        if (v > std::numeric_limits<int>::max()) return std::numeric_limits<int>::max();
        if (v < 0) return 0;
        return (int)v;
    }

    struct RGBA8 { unsigned char r, g, b, a; };

    // CAUTION: modifies the original soft image's palette!
    bool GetPalette256_(int soft, std::array<RGBA8, 256>& out)
    {
        for (int i = 0; i < 256; ++i)
        {
            int r = 0, g = 0, b = 0, a = 255;
            if (::DxLib::GetPaletteSoftImage(soft, i, &r, &g, &b, &a) != 0) return false;
            out[(size_t)i] = RGBA8{ (unsigned char)r,(unsigned char)g,(unsigned char)b,(unsigned char)a };
        }
        return true;
    }

    void SetPalette256_(int soft, const std::array<RGBA8, 256>& pal)
    {
        for (int i = 0; i < 256; ++i)
        {
            const auto& c = pal[(size_t)i];
            ::DxLib::SetPaletteSoftImage(soft, i, c.r, c.g, c.b, c.a);
        }
    }

    // NES style fade variant soft image
    void MakeFadePaletteMul_(const std::array<RGBA8, 256>& base, int variantIndex, int variantCount, std::array<RGBA8, 256>& out)
    {
        const int V = std::max(variantCount - 1, 1);
        const float s = 1.0f - (std::clamp(variantIndex, 0, V) / float(V));
        for (size_t i = 0; i < 256; ++i)
        {
            const int r = int(std::lround(base[i].r * s));
            const int g = int(std::lround(base[i].g * s));
            const int b = int(std::lround(base[i].b * s));
            out[i] = RGBA8{
                (unsigned char)std::clamp(r, 0, 255),
                (unsigned char)std::clamp(g, 0, 255),
                (unsigned char)std::clamp(b, 0, 255),
                base[i].a
            };
            if (i == 0) out[i].a = 0; // Transparent for index 0.
        }
    }
}

namespace mm2hack::apps::rendering::sprite
{
    SpriteCatalog::~SpriteCatalog()
    {
        for (Id id = 0; id < _atlases.size(); ++id)
        {
            if (_atlases[id] && _events.on_destroyed)
                _events.on_destroyed(id, _atlases[id]->Name());
        }
        _atlases.clear();
        _name_to_id.clear();
    }

    SpriteCatalog::Id SpriteCatalog::Load(const std::wstring& name, const std::wstring& png_path, const std::wstring& json_path)
    {
        auto it = _name_to_id.find(name);
        if (it != _name_to_id.end())
        {
            return it->second; // already loaded
        }

        const std::wstring png{ png_path };
        const std::wstring json{ json_path };
        auto atlas = BuildAtlas_(name, png, json);

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

        if (_events.on_created) _events.on_created(id, name);
        return id;
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

    int SpriteCatalog::MaxVariantAcross() const noexcept
    {
        int result = -1;
        for (const auto& p : _atlases)
        {
            if (!p) continue;
            const int mv = std::max(0, p->VariantCount() - 1);
            result = (result < 0) ? mv : std::min(result, mv);
        }
        return std::max(0, result);
    }

    SpriteCatalog::Id SpriteCatalog::NextId_() const noexcept
    {
        return static_cast<Id>(_atlases.size());
    }

    std::unique_ptr<SpriteAtlas> SpriteCatalog::BuildAtlas_(const std::wstring& name,
                                                            const std::wstring& png_path,
                                                            const std::wstring& json_path)
    {
        // --- JSON meta (supports both top-level and "Loader" wrapper) ---
        SpriteAtlas::DivSettings div{};
        SpriteAtlas::PaletteConfig pal{};

        if (!json_path.empty())
        {
            std::ifstream ifs(json_path);
            if (ifs)
            {
                nlohmann::json j; ifs >> j;
                if (j.is_null())
                {
                    THROW_EXCEPTION(L"JSON parse failed", kClassName);
                }
                else if (j.contains("Loader"))
                {
                    const auto& loader = j["Loader"];
                    div.tile_w = loader.value("tileWidth", div.tile_w);
                    div.tile_h = loader.value("tileHeight", div.tile_h);
                    div.tiles_x = loader.value("tilesX", div.tiles_x);
                    div.tiles_y = loader.value("tilesY", div.tiles_y);
                    pal.variant_count = loader.value("paletteVariants", pal.variant_count);
                    pal.nes_fade_step = loader.value("nesFadeStep", pal.nes_fade_step);
                }
            }
        }
        if (pal.variant_count <= 0) pal.variant_count = 1;

        if (div.tile_w <= 0 || div.tile_h <= 0 || div.tiles_x <= 0 || div.tiles_y <= 0)
        {
            THROW_EXCEPTION(L"Invalid sprite tileset div settings in json: " + json_path, kClassName);
        }

        // --- Load PNG as soft image ---
        if (png_path.empty())
        {
            THROW_EXCEPTION(L"The PNG file path is empty: " + png_path, kClassName);
        }
        std::ifstream png_ifs(png_path);
        if (!png_ifs)
        {
            THROW_EXCEPTION(L"File not found: " + png_path, kClassName);
        }

        const int soft = ::DxLib::LoadSoftImage(png_path.c_str());
        if (soft == -1)
        {
            THROW_EXCEPTION(L"LoadSoftImage failed: " + png_path, kClassName);
        }

        const int frame_count = mul_safe(div.tiles_x, div.tiles_y);
        std::vector<std::vector<int>> graphs_by_variant;
        graphs_by_variant.reserve(static_cast<std::size_t>(pal.variant_count));

        // Set up palettes
        std::array<RGBA8, 256> basePal{}, workPal{};
        const bool hasPal = GetPalette256_(soft, basePal);

        for (int v = 0; v < pal.variant_count; ++v)
        {
            std::vector<int> frames(static_cast<std::size_t>(frame_count), -1);

            if (hasPal)
            {
                // --- Change palette and then split ---
                MakeFadePaletteMul_(basePal, v, pal.variant_count, workPal);
                SetPalette256_(soft, workPal);

                const int created = ::DxLib::CreateDivGraphFromSoftImage(
                    soft,
                    div.tiles_x * div.tiles_y,
                    div.tiles_x, div.tiles_y,
                    div.tile_w, div.tile_h,
                    frames.data());

                if (created == -1)
                {
                    // fallback: split one by one
                    for (int ty = 0; ty < div.tiles_y; ++ty)
                    {
                        for (int tx = 0; tx < div.tiles_x; ++tx)
                        {
                            const int idx = ty * div.tiles_x + tx;
                            const int sx = tx * div.tile_w;
                            const int sy = ty * div.tile_h;
                            frames[(size_t)idx] = ::DxLib::CreateGraphFromRectSoftImage(soft, sx, sy, div.tile_w, div.tile_h);
                        }
                    }
                }
            }
            else
            {
                // TrueColor: If make NES style fade, first split normally, then darken by HSB (v=max is black)
                const int created = ::DxLib::CreateDivGraphFromSoftImage(soft, div.tiles_x * div.tiles_y, div.tiles_x, div.tiles_y, div.tile_w, div.tile_h, frames.data());

                if (created == -1)
                {
                    for (int ty = 0; ty < div.tiles_y; ++ty)
                    {
                        for (int tx = 0; tx < div.tiles_x; ++tx)
                        {
                            const int idx = ty * div.tiles_x + tx;
                            const int sx = tx * div.tile_w;
                            const int sy = ty * div.tile_h;
                            frames[(size_t)idx] = ::DxLib::CreateGraphFromRectSoftImage(soft, sx, sy, div.tile_w, div.tile_h);
                        }
                    }
                }

                if (pal.variant_count > 1)
                {
                    const int V = pal.variant_count - 1;
                    // v=0 -> bri=0 (brightest) / v=V -> bri=-255 (darkest)
                    const int bri = (V > 0) ? -int(std::lround((v / float(V)) * 255.0f)) : 0;
                    if (bri != 0)
                    {
                        for (int& h : frames) if (h != -1)
                        {
                            ::DxLib::GraphFilter(h, DX_GRAPH_FILTER_HSB, 0, 0, 0, bri);
                        }
                    }
                }
            }

            graphs_by_variant.emplace_back(std::move(frames));
        }

        // Turn back to base palette for safety.
        if (hasPal) SetPalette256_(soft, basePal);

        return std::make_unique<SpriteAtlas>(name, div, soft, std::move(graphs_by_variant));
    }
}