//==============================================================================
// 
//  Project: mm2hack
//  ICheatMemoryMap.h
// 
//  Cheat memory mapping interface.
// 
//==============================================================================
#pragma once

#include <cstdint>
#include <functional>
#include <string>

namespace mm2hack::core::cheats
{
    // Represents a memory location that can be read from and written to, along with a debug name
    struct ByteLocation
    {
        std::function<uint64_t()> read;         // Function to read the value at this location
        std::function<void(uint64_t)> write;    // Function to write a value to this location
        std::wstring debug_name;                // Optional debug name for this location
    };

    // Interface for resolving addresses and symbolic paths to ByteLocation instances
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