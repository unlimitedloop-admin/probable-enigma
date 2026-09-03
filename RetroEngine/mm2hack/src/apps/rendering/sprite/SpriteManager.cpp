#include "pch.h"

#include "SpriteManager.h"

#include <string_view>

namespace mm2hack::apps::rendering::sprite
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

    void SpriteManager::UseByName(const std::wstring& name, int frame, int x, int y)
    {
        const Id id = cacheId_(name);
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

    bool SpriteManager::ReplacePixelColorsById(
        Id id,
        std::span<const SpriteAtlas::PaletteColorMapping> mappings,
        int variant)
    {
        if (!_catalog.IsValid(id)) return false;
        return _catalog.GetAtlas(id).ReplacePixelColors(variant, mappings);
    }

    bool SpriteManager::ApplyRandomColorFilterByName(const std::wstring& name, int variant)
    {
        const Id id = cacheId_(name);
        if (id == kInvalidId || !_catalog.IsValid(id)) return false;
        return _catalog.GetAtlas(id).ApplyRandomHueToVariant(variant);
    }

    int SpriteManager::VariantCountById(Id id) const
    {
        if (!_catalog.IsValid(id)) return 0; return _catalog.GetAtlas(id).VariantCount();
    }

    void SpriteManager::SetGlobalVariantClamped(int v) noexcept
    {
        // clamp to [0, MaxVariant()]
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

    SpriteManager::Id SpriteManager::cacheId_(const std::wstring& name)
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
