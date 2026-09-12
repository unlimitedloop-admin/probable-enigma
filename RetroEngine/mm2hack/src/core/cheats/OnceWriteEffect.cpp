#include "pch.h"

#include "OnceWriteEffect.h"

#include "ICheatMemoryMap.h"

namespace mm2hack::core::cheats
{
    OnceWriteEffect::OnceWriteEffect(ByteLocation loc, uint64_t value, std::optional<uint64_t> cmp, std::wstring label)
        : _loc(std::move(loc)), _value(value), _compare(cmp), _label(std::move(label))
    {
    }

    void OnceWriteEffect::ApplyOnce()
    {
        if (_compare.has_value() && _loc.read() != _compare.value())
        {
            return;
        }
        _original = _loc.read();
        _loc.write(_value);
        _applied = true;
    }

    void OnceWriteEffect::ApplyFreeze()
    {
        // no-op
    }

    void OnceWriteEffect::Revert()
    {
        if (_applied && _original.has_value())
        {
            _loc.write(_original.value());
        }
    }

    bool OnceWriteEffect::IsFreeze() const
    {
        return false;
    }

    const std::wstring& OnceWriteEffect::GetLabel() const
    {
        return _label;
    }
}