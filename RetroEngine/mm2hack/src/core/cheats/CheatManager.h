//==============================================================================
// 
//  Project: mm2hack
//  CheatManager.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include <memory>
#include <string>
#include <vector>
#include "ICheatEffect.h"
#include "ICheatMemoryMap.h"
#include "ICodeParser.h"

namespace mm2hack::core::cheats
{
    struct CheatEntry
    {
        std::wstring code_string;
        std::unique_ptr<ICheatEffect> effect;
        bool enabled{ true };
    };

    class CheatManager
    {
    public:
        explicit CheatManager(ICheatMemoryMap& memory_map);

        // Provide parsers from outside (DI). Order matters; first-parse wins.
        void SetParsers(std::vector<std::unique_ptr<ICodeParser>> parsers);

        // UI entrypoint
        bool AddFromString(const std::wstring& code, std::wstring& out_error);

        // Lifecycle hooks
        void OnEnableAll();
        void OnDisableAll();
        void OnPreUpdate();
        void OnLateUpdate();

        const std::vector<CheatEntry>& GetEntries() const;

    private:
        ICheatMemoryMap& _memory_map;
        std::vector<std::unique_ptr<ICodeParser>> _parsers;
        std::vector<CheatEntry> _entries;
    };
}