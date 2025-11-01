//==============================================================================
// 
//  Project: mm2hack
//  BGTileCatalog.h
// 
//  The collection of BG tile atlases.
// 
//==============================================================================
#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>
#include "BGTileAtlas.h"

namespace mm2hack::apps::rendering::bg
{
    // Catalog of BG tile atlases
    class BGTileCatalog
    {
    public:
        using Id = std::uint32_t;

        struct Events
        {
            std::function<void(Id, const std::wstring&)> on_created{};   // after load
            std::function<void(Id, const std::wstring&)> on_destroyed{}; // before remove
        };

        BGTileCatalog() = default;
        ~BGTileCatalog();
        BGTileCatalog(const BGTileCatalog&) = delete;
        BGTileCatalog& operator=(const BGTileCatalog&) = delete;
        BGTileCatalog(BGTileCatalog&&) noexcept = default;
        BGTileCatalog& operator=(BGTileCatalog&&) noexcept = default;

        void SetEvents(Events e) noexcept { _events = std::move(e); }

        // Load BG tileset from PNG+JSON
        Id Load(const std::wstring& name, std::wstring_view png_path, std::wstring_view json_path);

        // Utilities
        [[nodiscard]] bool Has(const std::wstring& name) const;
        [[nodiscard]] Id GetId(const std::wstring& name) const;                 // throws if missing
        [[nodiscard]] std::optional<Id> TryGetId(const std::wstring& name) const noexcept; // nullopt

        // Access by Id (no bounds check)
        [[nodiscard]] const BGTileAtlas& GetAtlas(Id id) const noexcept;
        [[nodiscard]] BGTileAtlas& GetAtlas(Id id) noexcept;
        [[nodiscard]] bool IsValid(Id id) const noexcept;

        // Remove by Id
        void Remove(Id id);
        // Clear all
        void Clear();

        // Count
        [[nodiscard]] std::size_t Size() const noexcept { return _atlases.size(); }

        [[nodiscard]] int MaxVariantAcross() const noexcept;

    private:
        Id NextId_() const noexcept;        // next available Id (dense array index)
        std::unique_ptr<BGTileAtlas> BuildAtlas_(const std::wstring& name,
                                                 const std::wstring& png_path,
                                                 const std::wstring& json_path);    // Load to memory and build BG tile graphics

    private:
        const std::wstring kClassName{ L"BGTileCatalog" };

        std::unordered_map<std::wstring, Id> _name_to_id{};
        std::vector<std::unique_ptr<BGTileAtlas>> _atlases{}; // dense: index==Id
        Events _events{};
    };
}