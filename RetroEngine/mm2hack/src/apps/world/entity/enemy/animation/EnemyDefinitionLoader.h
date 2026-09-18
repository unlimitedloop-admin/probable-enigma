//==============================================================================
//
//  Project: mm2hack
//  EnemyDefinitionLoader.h
//
//  Reads and fully validates one enemy's JSON definition file (animation
//  state graph + palette presets) into an EnemyDefinition. All validation
//  happens here, once, at load time -- everything downstream (AnimationStatePlayer,
//  EnemyEntity, DemoStage2's palette-variant loading) can trust the result
//  without re-checking it.
//
//==============================================================================
#pragma once

#include <string>
#include <string_view>

#include "AnimationTypes.h"

namespace mm2hack::apps::world::entity::enemy::animation
{
    class EnemyDefinitionLoader final
    {
    public:
        // Reads a UTF-8 JSON file from disk and parses/validates it.
        static bool LoadFromFile(const std::wstring& filepath, EnemyDefinition& out);
        // Parses/validates UTF-8 JSON already in memory (no file access) --
        // exposed separately so it can be exercised without touching disk.
        static bool LoadFromJson(std::string_view source, EnemyDefinition& out);
    };
}
