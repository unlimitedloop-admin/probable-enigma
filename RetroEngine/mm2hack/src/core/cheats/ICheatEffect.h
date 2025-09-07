//==============================================================================
// 
//  Project: mm2hack
//  ICheatEffect.h
// 
//  Cheat effect interface.
// 
//==============================================================================
#pragma once

#include <string>

namespace mm2hack::core::cheats
{
    // Interface for cheat effects that can be applied, frozen, and reverted
    class ICheatEffect
    {
    public:
        virtual ~ICheatEffect() = default;
        // Execute once at OnEnable (warp, unlock, etc.)
        virtual void ApplyOnce() = 0;
        // Execute every LateUpdate when enabled (freeze HP, etc.)
        virtual void ApplyFreeze() = 0;
        // Restore original value if possible
        virtual void Revert() = 0;
        // Whether this effect is a "freeze" type
        virtual bool IsFreeze() const = 0;
        // Get the label/description of this effect
        virtual const std::wstring& GetLabel() const = 0;
    };
}