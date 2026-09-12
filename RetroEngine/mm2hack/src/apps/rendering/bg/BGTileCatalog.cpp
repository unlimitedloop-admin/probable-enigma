#include "pch.h"

#include "BGTileCatalog.h"

#include <string_view>
#include "apps/rendering/common/DividedGraphLoader.h"
#include "BGTileAtlas.h"

namespace mm2hack::apps::rendering::bg
{
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
        if (auto it = _name_to_id.find(name); it != _name_to_id.end())
        {
            return it->second;
        }
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

    BGTileCatalog::Id BGTileCatalog::Load(const std::wstring& name, std::wstring_view png_path, std::wstring_view json_path)
    {
        auto it = _name_to_id.find(name);
        if (it != _name_to_id.end())
        {
            return it->second; // already loaded
        }

        const std::wstring png{ png_path };
        const std::wstring json{ json_path };
        auto atlas = buildAtlas_(name, png, json);

        const Id id = nextId_();
        _atlases.emplace_back(std::move(atlas));
        
        _name_to_id.emplace(name, id);

        return id;
    }

    void BGTileCatalog::Remove(Id id)
    {
        if (!IsValid(id)) return;
        for (auto it = _name_to_id.begin(); it != _name_to_id.end(); )
        {
            if (it->second == id) it = _name_to_id.erase(it); else ++it;
        }
        _atlases[id].reset(); // dtor frees DxLib resources
    }

    void BGTileCatalog::Clear()
    {
        _atlases.clear();
        _name_to_id.clear();
    }

    int BGTileCatalog::MaxVariantAcross() const noexcept
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

    BGTileCatalog::Id BGTileCatalog::nextId_() const noexcept
    {
        return static_cast<Id>(_atlases.size());
    }

    std::unique_ptr<BGTileAtlas> BGTileCatalog::buildAtlas_(const std::wstring& name,
                                                            const std::wstring& png_path,
                                                            const std::wstring& json_path)
    {
        auto data = common::load_divided_graph(
            png_path, json_path, L"BG tileset", kClassName);
        return std::make_unique<BGTileAtlas>(
            name, data.div, data.soft_image, std::move(data.graphs_by_variant));
    }
}
