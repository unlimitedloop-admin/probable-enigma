#include "pch.h"

#include "FreezePatchEffect.h"

#include <cstdint>
#include <optional>
#include <utility>
#include "ICheatMemoryMap.h"

namespace mm2hack::core::cheats
{
    FreezePatchEffect::FreezePatchEffect(ByteLocation loc, uint64_t value, std::optional<uint64_t> cmp, std::wstring label)
        : _loc(std::move(loc)), _value(value), _compare(cmp), _label(std::move(label))
    {
    }

    void FreezePatchEffect::ApplyOnce()
    {
        if (_compare.has_value())
        {
            if (_loc.read() != _compare.value())
            {
                _applied = false;
                return;
            }
        }
        _original = _loc.read();
        _loc.write(_value);
        _applied = true;
    }

    void FreezePatchEffect::ApplyFreeze()
    {
        if (!_applied)
        {
            ApplyOnce();
        }
        _loc.write(_value);
    }

    void FreezePatchEffect::Revert()
    {
        if (_original.has_value())
        {
            _loc.write(_original.value());
        }
        _applied = false;
    }

    bool FreezePatchEffect::IsFreeze() const
    {
        return true;
    }

    const std::wstring& FreezePatchEffect::GetLabel() const
    {
        return _label;
    }
}