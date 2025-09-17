//==============================================================================
// 
//  Project: mm2hack
//  IWatchRegistry.h
// 
//  Memory watch function for monitoring game content in real time.
// 
//==============================================================================
#pragma once

#include <functional>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace mm2hack::core::diagnostics
{
    using WatchSupplier = std::function<std::wstring()>;
    struct WatchEntry { std::wstring name; std::wstring value; };

    // Interface for memory watch registry
    class IWatchRegistry
    {
    public:
        virtual ~IWatchRegistry() = default;

        // Register / Unregister / Contains
        virtual void Register(const std::wstring& name, WatchSupplier supplier) = 0;
        virtual void Unregister(const std::wstring& name) = 0;
        virtual bool Contains(const std::wstring& name) const = 0;

        // Get memory watch snapshot all
        virtual std::vector<WatchEntry> SnapshotAll() const = 0;
        // Get memory watch snapshot by prefix
        virtual std::vector<WatchEntry> SnapshotPrefix(const std::wstring& prefix) const = 0;
        // Remove all memory watches
        virtual void Clear() = 0;
    };

    // Utility functions to convert various types -> std::wstring
    inline std::wstring ToWString(const std::wstring& v) { return v; }
    inline std::wstring ToWString(const std::string& v) { return std::wstring(v.begin(), v.end()); }
    inline std::wstring ToWString(const wchar_t* v) { return v ? std::wstring(v) : L""; }
    inline std::wstring ToWString(const char* v) { return v ? ToWString(std::string(v)) : L""; }
    inline std::wstring ToWString(bool v) { return v ? L"true" : L"false"; }
    template<class T> requires std::is_arithmetic_v<T>
    inline std::wstring ToWString(T v) { return std::to_wstring(v); }
    template<class T>
    inline std::wstring ToWString(const T& v) { std::wostringstream oss; oss << v; return oss.str(); }
}