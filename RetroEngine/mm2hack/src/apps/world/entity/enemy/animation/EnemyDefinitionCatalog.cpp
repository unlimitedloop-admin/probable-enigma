#include "pch.h"

#include "EnemyDefinitionCatalog.h"

#include "EnemyDefinitionLoader.h"

namespace mm2hack::apps::world::entity::enemy::animation
{
    bool EnemyDefinitionCatalog::LoadForKind(EnemyKind kind, const std::wstring& filepath)
    {
        EnemyDefinition parsed{};
        if (!EnemyDefinitionLoader::LoadFromFile(filepath, parsed))
        {
            return false;
        }

        _definitions[kind] = std::move(parsed);
        return true;
    }

    const EnemyDefinition* EnemyDefinitionCatalog::Find(EnemyKind kind) const noexcept
    {
        const auto it = _definitions.find(kind);
        return it != _definitions.end() ? &it->second : nullptr;
    }
}
