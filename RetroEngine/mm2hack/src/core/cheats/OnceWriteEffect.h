//==============================================================================
// 
//  Project: mm2hack
//  OnceWriteEffect.h
// 
//  ** Descriptions **
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
    class OnceWriteEffect final : public ICheatEffect
    {
    public:
        OnceWriteEffect(ByteLocation loc, uint64_t value, std::optional<uint64_t> cmp, std::wstring label);
        ~OnceWriteEffect() = default;

        void ApplyOnce() override;
        void ApplyFreeze() override;
        void Revert() override;
        bool IsFreeze() const override;
        const std::wstring& GetLabel() const override;

    private:
        ByteLocation _loc;
        uint64_t _value{ 0 };
        std::optional<uint64_t> _compare;
        std::optional<uint64_t> _original;
        bool _applied{ false };
        std::wstring _label;
    };
}