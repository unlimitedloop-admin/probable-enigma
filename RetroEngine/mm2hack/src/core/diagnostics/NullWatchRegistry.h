//==============================================================================
// 
//  Project: mm2hack
//  NullWatchRegistry.h
// 
//  Null implementation of the IWatchRegistry interface, providing no-op behavior.
// 
//==============================================================================
#pragma once

#include "IWatchRegistry.h"

#include <string>
#include <vector>

namespace mm2hack::core::diagnostics
{
    // NullWatchRegistry class that implements IWatchRegistry with empty methods
    class NullWatchRegistry final : public IWatchRegistry
    {
    public:
        void Register(const std::wstring&, WatchSupplier) override {}
        void Unregister(const std::wstring&) override {}
        bool Contains(const std::wstring&) const override { return false; }
        std::vector<WatchEntry> SnapshotAll() const override { return {}; }
        std::vector<WatchEntry> SnapshotPrefix(const std::wstring&) const override { return {}; }
        void Clear() override {}

    private:
        const std::wstring kClassName = L"NullWatchRegistry";
    };
}