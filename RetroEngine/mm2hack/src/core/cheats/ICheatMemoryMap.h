//==============================================================================
// 
//  Project: mm2hack
//  ICheatMemoryMap.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include <cstdint>
#include <functional>
#include <string>

namespace mm2hack::core::cheats
{
    struct ByteLocation
    {
        std::function<uint64_t()> read;
        std::function<void(uint64_t)> write;
        std::wstring debug_name;
    };

    class ICheatMemoryMap
    {
    public:
        virtual ~ICheatMemoryMap() = default;

        // Resolve PAR-style numeric address to a writable binding.
        virtual bool TryResolve(uint32_t address, ByteLocation& out) = 0;

        // Resolve "player.invincible" style symbolic path.
        virtual bool TryResolveSymbol(const std::wstring& path, ByteLocation& out) = 0;
    };
}