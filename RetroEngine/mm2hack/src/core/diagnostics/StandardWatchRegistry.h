//==============================================================================
// 
//  Project: mm2hack
//  StandardWatchRegistry.h
// 
//  Memory watch registry implementation.
// 
//==============================================================================
#pragma once

#include "IWatchRegistry.h"

#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace mm2hack::core::diagnostics
{
    // Memory registration for monitoring game content in real time
    class StandardWatchRegistry final : public IWatchRegistry
    {
    public:
        StandardWatchRegistry() = default;
        ~StandardWatchRegistry() override = default;

        // Register to memory variables
        void Register(const std::wstring& name, WatchSupplier s) override;
        // Unregister from memory variables
        void Unregister(const std::wstring& name) override;
        // Check if the memory variable is registered
        bool Contains(const std::wstring& name) const override;
        // Get memory watch snapshot all
        std::vector<WatchEntry> SnapshotAll() const override;
        // Get memory watch snapshot by prefix
        std::vector<WatchEntry> SnapshotPrefix(const std::wstring& prefix) const override;
        // Remove all memory watches
        void Clear() override;

    private:
        const std::wstring kClassName = L"StandardWatchRegistry";

        mutable std::shared_mutex _mtx;     // Mutex for thread safety
        std::unordered_map<std::wstring, WatchSupplier> _map;   // Map of registered memory variables
    };
}