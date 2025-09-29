#include "pch.h"

#include "SpriteManager.h"

#include <string_view>

namespace mm2hack::apps::graphics::sprite
{
    SpriteManager::Id SpriteManager::Load(const std::wstring& name, const std::wstring_view png_path, const std::wstring_view json_path)
    {
        const std::wstring png = std::wstring(png_path);
        const std::wstring json = std::wstring(json_path);

        return _catalog.Load(name, png, json);
    }

    void SpriteManager::UseById(Id id, int frame, int x, int y) const noexcept
    {
        if (!_catalog.IsValid(id)) { return; }
        const auto& atlas = _catalog.GetAtlas(id);
        const int v = _global_variant;
        atlas.Draw(v, frame, x, y);
    }

    void SpriteManager::UseByIdVariant(Id id, int variant, int frame, int x, int y) const noexcept
    {
        if (!_catalog.IsValid(id)) { return; }
        const auto& atlas = _catalog.GetAtlas(id);
        atlas.Draw(variant, frame, x, y);
    }

    void SpriteManager::Use(const std::wstring& name, int frame, int x, int y)
    {
        const Id id = CacheId_(name);
        if (id == kInvalidId) { return; }
        UseById(id, frame, x, y);
    }

    bool SpriteManager::ReplacePaletteColorByName(const std::wstring& name, int targetPaletteIndex, int sourcePaletteIndex, int variant)
    {
        if (auto it = _name_cache.find(name); it != _name_cache.end())
        {
            return _catalog.GetAtlas(it->second).ReplacePaletteColorIndex(variant, targetPaletteIndex, sourcePaletteIndex);
        }
        if (auto opt = _catalog.TryGetId(name))
        {
            _name_cache.emplace(name, *opt);
            return _catalog.GetAtlas(*opt).ReplacePaletteColorIndex(variant, targetPaletteIndex, sourcePaletteIndex);
        }
        return false;
    }

    bool SpriteManager::ReplacePaletteColorById(Id id, int targetPaletteIndex, int sourcePaletteIndex, int variant)
    {
        if (!_catalog.IsValid(id)) return false;
        return _catalog.GetAtlas(id).ReplacePaletteColorIndex(variant, targetPaletteIndex, sourcePaletteIndex);
    }

    bool SpriteManager::ApplyRandomColorFilterByName(const std::wstring& name, int variant)
    {
        const Id id = CacheId_(name);
        if (id == kInvalidId || !_catalog.IsValid(id)) return false;
        return _catalog.GetAtlas(id).ApplyRandomHueToVariant(variant);
    }

    bool SpriteManager::ApplyRandomColorFilterById(Id id, int variant)
    {
        if (!_catalog.IsValid(id)) return false;
        return _catalog.GetAtlas(id).ApplyRandomHueToVariant(variant);
    }

    bool SpriteManager::ApplyHSBFilterById(Id id, int variant, int hueAdd, int satAdd, int briAdd)
    {
        if (!_catalog.IsValid(id)) return false;
        return _catalog.GetAtlas(id).ApplyHSBToVariant(variant, hueAdd, satAdd, briAdd);
    }

    int SpriteManager::VariantCountByName(const std::wstring& sprite_name) const
    {
        if (auto id = _catalog.TryGetId(sprite_name)) return _catalog.GetAtlas(*id).VariantCount();
        return 0;
    }

    int SpriteManager::VariantCountById(Id id) const
    {
        if (!_catalog.IsValid(id)) return 0; return _catalog.GetAtlas(id).VariantCount();
    }

    void SpriteManager::SetGlobalVariantClamped(int v) noexcept
    {
        const int mv = MaxVariant();
        _global_variant = std::max(0, std::min(v, mv));
    }

    void SpriteManager::ReleaseById(Id id)
    {
        // drop any cached name entries pointing to this id
        for (auto it = _name_cache.begin(); it != _name_cache.end();)
        {
            if (it->second == id) it = _name_cache.erase(it); else ++it;
        }
        _catalog.Remove(id);
    }

    void SpriteManager::ReleaseByName(const std::wstring& name)
    {
        if (auto it = _name_cache.find(name); it != _name_cache.end())
        {
            _name_cache.erase(it);
        }
        if (auto opt = _catalog.TryGetId(name))
        {
            _catalog.Remove(*opt);
        }
    }

    void SpriteManager::ReleaseAll()
    {
        _catalog.Clear();
        _name_cache.clear();
        _global_variant = 0;
    }

    SpriteManager::Id SpriteManager::CacheId_(const std::wstring& name)
    {
        if (const auto it = _name_cache.find(name); it != _name_cache.end())
        {
            return it->second;
        }
        if (auto opt = _catalog.TryGetId(name))
        {
            const Id id = *opt;
            _name_cache.emplace(name, id);
            return id;
        }
        return kInvalidId;
    }
}