//==============================================================================
// 
//  Project: mm2hack
//  CheatManager.h
// 
//  Applys and manages cheat codes.
// 
//==============================================================================
#pragma once

#include <memory>
#include <string>
#include <vector>
#include "ICheatEffect.h"
#include "ICodeParser.h"

namespace mm2hack::core::cheats
{
    class ICheatMemoryMap;
}

namespace mm2hack::core::cheats
{
    // A single cheat entry, consisting of the original code string, the parsed effect, and its enabled state
    struct CheatEntry
    {
        std::wstring code_string;
        std::unique_ptr<ICheatEffect> effect;
        bool enabled{ true };
    };

    // Manages cheat codes: parsing, storing, enabling/disabling, and applying effects
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

        // Enable/disable individual cheats
        const std::vector<CheatEntry>& GetEntries() const;

    private:
        const std::wstring kClassName = L"CheatManager";

        ICheatMemoryMap& _memory_map;                           // Reference to the memory map for resolving addresses
        std::vector<std::unique_ptr<ICodeParser>> _parsers;     // List of parsers for interpreting cheat codes
        std::vector<CheatEntry> _entries;                       // List of all cheat entries
    };
}