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
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "SpriteAtlas.h"

namespace mm2hack::apps::rendering::sprite
{
    // Manages multiple sprite atlases, identified by name and Id
    class SpriteCatalog
    {
    public:
        using Id = std::uint32_t;

        struct Events
        {
            std::function<void(Id, const std::wstring&)> on_created{};   // after load
            std::function<void(Id, const std::wstring&)> on_destroyed{}; // before remove
        };

        SpriteCatalog() = default;
        ~SpriteCatalog();

        SpriteCatalog(const SpriteCatalog&) = delete;
        SpriteCatalog& operator=(const SpriteCatalog&) = delete;
        SpriteCatalog(SpriteCatalog&&) noexcept = default;
        SpriteCatalog& operator=(SpriteCatalog&&) noexcept = default;

        // Set event callbacks used for monitoring
        void SetEvents(Events events) noexcept { _events = std::move(events); }

        // Load from PNG + JSON metadata (div settings, optional palette variants)
        Id Load(const std::wstring& name, const std::wstring& png_path, const std::wstring& json_path);

        // Getters
        bool Has(const std::wstring& name) const;
        Id GetId(const std::wstring& name) const;
        std::optional<Id> TryGetId(const std::wstring& name) const noexcept;
        const SpriteAtlas& GetAtlas(Id id) const noexcept;
        SpriteAtlas& GetAtlas(Id id) noexcept;

        // Check if the atlas is valid
        bool IsValid(Id id) const noexcept;

        // release atlas and its graphs
        void Remove(Id id);
        void Clear();

        // Count
        [[nodiscard]] std::size_t Size() const noexcept { return _atlases.size(); }
        // Maximum variant count across all loaded atlases
        [[nodiscard]] int MaxVariantAcross() const noexcept;

    private:
        Id NextId_() const noexcept;        // next available Id (dense array index)
        std::unique_ptr<SpriteAtlas> BuildAtlas_(const std::wstring& name,
                                                 const std::wstring& png_path,
                                                 const std::wstring& json_path);    // Load to memory and build sprite graphics

    private:
        const std::wstring kClassName{ L"SpriteCatalog" };

        std::unordered_map<std::wstring, Id> _name_to_id{};     // name -> Id
        std::vector<std::unique_ptr<SpriteAtlas>> _atlases{};   // dense array; index==Id
        Events _events{};                                       // event callbacks (optional)
    };
}