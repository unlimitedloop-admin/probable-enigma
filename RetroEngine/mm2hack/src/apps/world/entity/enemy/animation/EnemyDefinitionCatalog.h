//==============================================================================
//
//  Project: mm2hack
//  EnemyDefinitionCatalog.h
//
//  Loads and owns each EnemyKind's EnemyDefinition (animation state graph +
//  palette presets) for the process's lifetime. Loaded once per stage
//  (DemoStage2::initializeResources_()); every EnemyEntity of that kind
//  afterward references the same instance through a raw, non-owning pointer.
//
//==============================================================================
#pragma once

#include <string_view>
#include <unordered_map>

#include "AnimationTypes.h"
#include "apps/world/entity/enemy/lists/EnemyLists.h"

namespace mm2hack::apps::world::entity::enemy::animation
{
    class EnemyDefinitionCatalog final
    {
    public:
        // Loads one definition file and registers it under `kind`. On any
        // parse/validation failure, returns false and leaves whatever was
        // previously registered for `kind` (if anything) untouched.
        bool LoadForKind(EnemyKind kind, const std::wstring& filepath);

        [[nodiscard]] const EnemyDefinition* Find(EnemyKind kind) const noexcept;
        // Looks a loaded definition up by its JSON "id" (e.g. "metall") -- how
        // stage placement data names a kind. Also reports the kind it's
        // registered under. nullptr if no loaded definition has that id.
        [[nodiscard]] const EnemyDefinition* FindById(std::string_view id, EnemyKind& out_kind) const noexcept;

        // Drops every loaded definition (e.g. between stage loads).
        void Clear() noexcept { _definitions.clear(); }

    private:
        std::unordered_map<EnemyKind, EnemyDefinition> _definitions;
    };
}
