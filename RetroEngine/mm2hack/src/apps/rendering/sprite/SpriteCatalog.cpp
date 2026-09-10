#include "pch.h"

#include "SpriteCatalog.h"

#include "apps/rendering/common/DividedGraphLoader.h"
#include "SpriteAtlas.h"

namespace mm2hack::apps::rendering::sprite
{
    SpriteCatalog::Id SpriteCatalog::Load(
        const std::wstring& name, const std::wstring& png_path,
        const std::wstring& json_path, bool* out_created)
    {
        auto it = _name_to_id.find(name);
        if (it != _name_to_id.end())
        {
            if (out_created) *out_created = false;
            return it->second; // already loaded
        }

        const std::wstring png{ png_path };
        const std::wstring json{ json_path };
        auto atlas = BuildAtlas_(png, json);

        const Id id = static_cast<Id>(_atlases.size());
        _atlases.emplace_back(std::move(atlas));
        _name_to_id.emplace(name, id);
        if (out_created) *out_created = true;

        return id;
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
        _atlases.clear();
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

    std::unique_ptr<SpriteAtlas> SpriteCatalog::BuildAtlas_(const std::wstring& png_path,
                                                            const std::wstring& json_path)
    {
        auto data = common::load_divided_graph(
            png_path, json_path, L"sprite tileset", kClassName);
        return std::make_unique<SpriteAtlas>(
            data.div, data.soft_image, std::move(data.graphs_by_variant));
    }
}
