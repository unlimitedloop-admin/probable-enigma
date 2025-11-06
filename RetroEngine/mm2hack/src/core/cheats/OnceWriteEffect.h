//==============================================================================
// 
//  Project: mm2hack
//  OnceWriteEffect.h
// 
//  Cheat effect for writing a value to memory once.
// 
//==============================================================================
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include "ICheatEffect.h"
#include "ICheatMemoryMap.h"

namespace mm2hack::core::cheats
{
    // An effect that writes a value to a memory location once, optionally comparing it first
    class OnceWriteEffect final : public ICheatEffect
    {
    public:
        OnceWriteEffect(ByteLocation loc, uint64_t value, std::optional<uint64_t> cmp, std::wstring label);
        ~OnceWriteEffect() = default;

        // Write the value to the memory location once (only if the comparison value is specified and matches)
        void ApplyOnce() override;
        // No-op for freeze effects
        void ApplyFreeze() override;
        // Revert to the original value (only if possible)
        void Revert() override;
        // This effect is not a freeze type
        bool IsFreeze() const override;
        // Get the label (description) of the effect
        const std::wstring& GetLabel() const override;

    private:
        const std::wstring kClassName = L"OnceWriteEffect";

        ByteLocation _loc;                  // Memory location to patch
        uint64_t _value{ 0 };               // Value to write
        std::optional<uint64_t> _compare;   // Optional comparison value
        std::optional<uint64_t> _original;  // Original value before patching
        bool _applied{ false };             // Whether the patch has been applied
        std::wstring _label;                // Description label for the effect
    };
}