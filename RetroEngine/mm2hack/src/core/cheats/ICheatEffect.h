//==============================================================================
// 
//  Project: mm2hack
//  ICheatEffect.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include <string>

namespace mm2hack::core::cheats
{
    class ICheatEffect
    {
    public:
        virtual ~ICheatEffect() = default;

        // Execute once at OnEnable (warp, unlock, etc.)
        virtual void ApplyOnce() = 0;

        // Execute every LateUpdate when enabled (freeze HP, etc.)
        virtual void ApplyFreeze() = 0;

        // Restore original value if possible.
        virtual void Revert() = 0;

        virtual bool IsFreeze() const = 0;
        virtual const std::wstring& GetLabel() const = 0;
    };
}