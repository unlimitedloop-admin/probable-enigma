#include "pch.h"

#include "SpriteManager.h"

#include <span>
#include <string_view>
#include "SpriteAtlas.h"

namespace mm2hack::apps::rendering::sprite
{
    SpriteManager::Id SpriteManager::Load(
        const std::wstring& name, const std::wstring_view png_path,
        const std::wstring_view json_path, bool* out_created)
    {
        const std::wstring png = std::wstring(png_path);
        const std::wstring json = std::wstring(json_path);
        return _catalog.Load(name, png, json, out_created);
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

    bool SpriteManager::ReplacePaletteColorById(Id id, int targetPaletteIndex, int sourcePaletteIndex, int variant)
    {
        if (!_catalog.IsValid(id)) return false;
        return _catalog.GetAtlas(id).ReplacePaletteColorIndex(variant, targetPaletteIndex, sourcePaletteIndex);
    }

    bool SpriteManager::ReplacePaletteColorsById(
        Id id, std::span<const SpriteAtlas::PaletteColorMapping> mappings)
    {
        if (!_catalog.IsValid(id)) return false;
        return _catalog.GetAtlas(id).ReplacePaletteColors(mappings);
    }

    bool SpriteManager::ReplacePixelColorsById(
        Id id,
        std::span<const SpriteAtlas::PaletteColorMapping> mappings,
        int variant)
    {
        if (!_catalog.IsValid(id)) return false;
        return _catalog.GetAtlas(id).ReplacePixelColors(variant, mappings);
    }

    bool SpriteManager::ApplyHueFilterById(Id id, int hue_add, int variant)
    {
        if (!_catalog.IsValid(id)) return false;
        return _catalog.GetAtlas(id).ApplyHSBToVariant(variant, hue_add, 0, 0);
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
        _catalog.Remove(id);
    }

    void SpriteManager::ReleaseAll()
    {
        _catalog.Clear();
        _global_variant = 0;
    }
}
