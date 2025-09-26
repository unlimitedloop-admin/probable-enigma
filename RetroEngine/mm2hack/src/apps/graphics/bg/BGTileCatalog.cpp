#include "pch.h"

#include "BGTileCatalog.h"

#include <limits>
#include <nlohmann/json.hpp>
#include <optional>
#include <string_view>
#include "BGTileAtlas.h"

namespace
{
    [[nodiscard]] int mul_safe(int a, int b)
    {
        long long v = static_cast<long long>(a) * static_cast<long long>(b);
        if (v > std::numeric_limits<int>::max()) return std::numeric_limits<int>::max();
        if (v < 0) return 0; return static_cast<int>(v);
    }

    // TODO: Implement palette-variant generation on SoftImage (same stub as sprites)
    int make_palette_variant_softimage(int base_soft, int /*variant_index*/, int /*fade_step*/)
    {
        return base_soft; // stub: reuse base
    }
}

namespace mm2hack::apps::graphics::bg
{
    BGTileCatalog::~BGTileCatalog()
    {
        for (Id id = 0; id < _atlases.size(); ++id)
        {
            if (_atlases[id] && _events.on_destroyed)
                _events.on_destroyed(id, _atlases[id]->Name());
        }
        _atlases.clear();
        _name_to_id.clear();
    }

    bool BGTileCatalog::Has(const std::wstring& name) const
    {
        return _name_to_id.find(name) != _name_to_id.end();
    }

    BGTileCatalog::Id BGTileCatalog::GetId(const std::wstring& name) const
    {
        return _name_to_id.at(name);
    }

    std::optional<BGTileCatalog::Id> BGTileCatalog::TryGetId(const std::wstring& name) const noexcept
    {
        if (auto it = _name_to_id.find(name); it != _name_to_id.end()) return it->second;
        return std::nullopt;
    }

    const BGTileAtlas& BGTileCatalog::GetAtlas(Id id) const noexcept
    {
        return *(_atlases[static_cast<std::size_t>(id)]);
    }

    BGTileAtlas& BGTileCatalog::GetAtlas(Id id) noexcept
    {
        return *(_atlases[static_cast<std::size_t>(id)]);
    }

    bool BGTileCatalog::IsValid(Id id) const noexcept
    {
        return id < _atlases.size() && static_cast<bool>(_atlases[id]);
    }

    BGTileCatalog::Id BGTileCatalog::NextId_() const noexcept
    {
        return static_cast<Id>(_atlases.size());
    }

    BGTileCatalog::Id BGTileCatalog::Load(const std::wstring& name, std::wstring_view png_path, std::wstring_view json_path)
    {
        if (auto it = _name_to_id.find(name); it != _name_to_id.end()) return it->second;

        const std::wstring png{ png_path };
        const std::wstring json{ json_path };
        auto atlas = BuildAtlas_(name, png, json);

        const Id id = NextId_();
        _atlases.emplace_back(std::move(atlas));
        _name_to_id.emplace(name, id);
        if (_events.on_created) _events.on_created(id, name);
        return id;
    }

    void BGTileCatalog::Remove(Id id)
    {
        if (!IsValid(id)) return;
        if (_events.on_destroyed) _events.on_destroyed(id, _atlases[id]->Name());
        for (auto it = _name_to_id.begin(); it != _name_to_id.end(); )
        {
            if (it->second == id) it = _name_to_id.erase(it); else ++it;
        }
        _atlases[id].reset();
    }

    void BGTileCatalog::Clear()
    {
        for (Id id = 0; id < _atlases.size(); ++id)
        {
            if (_atlases[id] && _events.on_destroyed)
            {
                _events.on_destroyed(id, _atlases[id]->Name());
            }
        }
        _atlases.clear();
        _name_to_id.clear();
    }

    std::unique_ptr<BGTileAtlas> BGTileCatalog::BuildAtlas_(const std::wstring& name, const std::wstring& png, const std::wstring& json)
    {
        // --- JSON meta (supports both top-level and "Loader" wrapper) ---
        BGTileAtlas::DivSettings div{};
        BGTileAtlas::PaletteConfig pal{};

        if (!json.empty())
        {
            std::ifstream ifs(json);
            if (ifs)
            {
                nlohmann::json root;
                ifs >> root;

                const nlohmann::json& meta = root.contains("Loader") ? root["Loader"] : root;
                div.tile_w = meta.value("tileWidth", div.tile_w);
                div.tile_h = meta.value("tileHeight", div.tile_h);
                div.tiles_x = meta.value("tilesX", div.tiles_x);
                div.tiles_y = meta.value("tilesY", div.tiles_y);
                pal.variant_count = meta.value("paletteVariants", pal.variant_count);
                pal.nes_fade_step = meta.value("nesFadeStep", pal.nes_fade_step);
            }
        }

        // Or not?
        if (div.tile_w <= 0 || div.tile_h <= 0 || div.tiles_x <= 0 || div.tiles_y <= 0)
        {
            THROW_EXCEPTION(L"Invalid BG tileset div settings in JSON.", L"BGTileCatalog");
        }

        const int soft = ::DxLib::LoadSoftImage(png.c_str());
        if (soft == -1)
        {
            THROW_EXCEPTION(L"LoadSoftImage failed for BG tileset", L"BGTileCatalog");
        }

        const int total = mul_safe(div.tiles_x, div.tiles_y);
        std::vector<std::vector<int>> graphs_by_variant;
        graphs_by_variant.reserve(static_cast<std::size_t>(pal.variant_count));

        for (int v = 0; v < pal.variant_count; ++v)
        {
            const int src_soft = make_palette_variant_softimage(soft, v, pal.nes_fade_step);
            std::vector<int> handles(static_cast<std::size_t>(total), -1);
            // DxLib signature: (SI, AllNum, XNum, YNum, XSize, YSize, out[])
            const int res = ::DxLib::CreateDivGraphFromSoftImage(src_soft, total, div.tiles_x, div.tiles_y, div.tile_w, div.tile_h, handles.data());
            if (res != 0)
            {
                // TODO: fallback per-tile creation if needed
            }
            graphs_by_variant.emplace_back(std::move(handles));
        }
        return std::make_unique<BGTileAtlas>(name, div, soft, std::move(graphs_by_variant));
    }
}