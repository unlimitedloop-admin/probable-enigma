//==============================================================================
//
//  Project: mm2hack
//  StageDefinitionLoader.h
//
//  Reads the placement-related parts of a stage's .def file (BD-006):
//  "stage.start", each node's "enemyRespawn" default, and the "entities"
//  of type "enemy". Everything else in the file (tileAttributes, events,
//  triggers, scripts, other entity types) is left for later readers.
//
//==============================================================================
#pragma once

#include <string>
#include <string_view>

#include "StagePlacement.h"

namespace mm2hack::apps::world::stage
{
    class StageDefinitionLoader final
    {
    public:
        // Returns false (with a human-readable reason in `error`) on a
        // malformed file; `out` is only written on success.
        static bool LoadFromFile(const std::wstring& filepath, StageDefinitionData& out, std::string& error);
        static bool LoadFromJson(std::string_view source, StageDefinitionData& out, std::string& error);
    };
}
