//==============================================================================
// 
//  Project: mm2hack
//  FreezePatchEffect.h
// 
//  Cheat effect for patching and freezing memory values.
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
    // An effect that patches a memory location with a value, optionally comparing it first, and can freeze it
    class FreezePatchEffect final : public ICheatEffect
    {
    public:
        FreezePatchEffect(ByteLocation loc, uint64_t value, std::optional<uint64_t> cmp, std::wstring label);
        ~FreezePatchEffect() = default;

        // Write the value to the memory location once (only if the comparison value is specified and matches)
        void ApplyOnce() override;
        // While enabled, forcibly write the value every frame, "freezing" it
        void ApplyFreeze() override;
        // Revert to the original value (only if possible)
        void Revert() override;
        // Determine if this effect is of the "freeze" type
        bool IsFreeze() const override;
        // Get the label (description) of the effect
        const std::wstring& GetLabel() const override;

    private:
        ByteLocation _loc;                  // Memory location to patch
        uint64_t _value{ 0 };               // Value to write
        std::optional<uint64_t> _compare;   // Optional comparison value
        std::optional<uint64_t> _original;  // Original value before patching
        bool _applied{ false };             // Whether the patch has been applied
        std::wstring _label;                // Description label for the effect
    };
}