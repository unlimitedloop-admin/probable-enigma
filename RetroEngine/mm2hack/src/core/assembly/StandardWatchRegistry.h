//==============================================================================
// 
//  Project: mm2hack
//  StandardWatchRegistry.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include "IWatchRegistry.h"

#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace mm2hack::core::assembly
{
    class StandardWatchRegistry final : public IWatchRegistry
    {
    public:
        StandardWatchRegistry() = default;
        ~StandardWatchRegistry() override = default;

        void Register(const std::wstring& name, WatchSupplier s) override;
        void Unregister(const std::wstring& name) override;
        bool Contains(const std::wstring& name) const override;
        std::vector<WatchEntry> SnapshotAll() const override;
        std::vector<WatchEntry> SnapshotPrefix(const std::wstring& prefix) const override;
        void Clear() override;

    private:
        mutable std::shared_mutex _mtx;
        std::unordered_map<std::wstring, WatchSupplier> _map;
    };
}