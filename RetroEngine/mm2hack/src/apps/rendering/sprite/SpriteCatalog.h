//==============================================================================
// 
//  Project: mm2hack
//  SpriteCatalog.h
// 
//  The collection of sprite atlases.
// 
//==============================================================================
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "SpriteAtlas.h"

namespace mm2hack::apps::rendering::sprite
{
    // Manages multiple sprite atlases, identified by name and Id
    class SpriteCatalog
    {
    public:
        using Id = std::uint32_t;

        SpriteCatalog() = default;
        ~SpriteCatalog() = default;

        SpriteCatalog(const SpriteCatalog&) = delete;
        SpriteCatalog& operator=(const SpriteCatalog&) = delete;
        SpriteCatalog(SpriteCatalog&&) noexcept = default;
        SpriteCatalog& operator=(SpriteCatalog&&) noexcept = default;

        // Load from PNG + JSON metadata (div settings, optional palette variants)
        Id Load(const std::wstring& name, const std::wstring& png_path, const std::wstring& json_path);

        // Getters
        const SpriteAtlas& GetAtlas(Id id) const noexcept;
        SpriteAtlas& GetAtlas(Id id) noexcept;

        // Check if the atlas is valid
        bool IsValid(Id id) const noexcept;

        // release atlas and its graphs
        void Remove(Id id);
        void Clear();

        // Maximum variant count across all loaded atlases
        [[nodiscard]] int MaxVariantAcross() const noexcept;

    private:
        std::unique_ptr<SpriteAtlas> BuildAtlas_(const std::wstring& png_path,
                                                 const std::wstring& json_path);    // Load to memory and build sprite graphics

    private:
        const std::wstring kClassName{ L"SpriteCatalog" };

        std::unordered_map<std::wstring, Id> _name_to_id{};     // name -> Id
        std::vector<std::unique_ptr<SpriteAtlas>> _atlases{};   // dense array; index==Id
    };
}
