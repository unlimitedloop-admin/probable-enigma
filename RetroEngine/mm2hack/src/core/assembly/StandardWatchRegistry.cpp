#include "pch.h"

#include "StandardWatchRegistry.h"

#include "IWatchRegistry.h"

namespace mm2hack::core::assembly
{
    void StandardWatchRegistry::Register(const std::wstring& name, WatchSupplier s)
    {
        std::unique_lock lock(_mtx);
        _map[name] = std::move(s);
    }

    void StandardWatchRegistry::Unregister(const std::wstring& name)
    {
        std::unique_lock lock(_mtx);
        _map.erase(name);
    }

    bool StandardWatchRegistry::Contains(const std::wstring& name) const
    {
        std::shared_lock lock(_mtx);
        return _map.find(name) != _map.end();
    }

    std::vector<WatchEntry> StandardWatchRegistry::SnapshotAll() const
    {
        std::vector<WatchEntry> out;
        std::shared_lock lock(_mtx);
        out.reserve(_map.size());
        for (const auto& [k, sup] : _map)
        {
            try { out.push_back({ k, sup ? sup() : L"" }); }
            catch (...) { out.push_back({ k, L"<supplier threw>" }); }
        }
        return out;
    }

    std::vector<WatchEntry> StandardWatchRegistry::SnapshotPrefix(const std::wstring& prefix) const
    {
        std::vector<WatchEntry> out;
        std::shared_lock lock(_mtx);
        for (const auto& [k, sup] : _map)
        {
            if (k.rfind(prefix, 0) == 0) // starts_with
            {
                try { out.push_back({ k, sup ? sup() : L"" }); }
                catch (...) { out.push_back({ k, L"<supplier threw>" }); }
            }
        }
        return out;
    }

    void StandardWatchRegistry::Clear()
    {
        std::unique_lock lock(_mtx);
        _map.clear();
    }
}