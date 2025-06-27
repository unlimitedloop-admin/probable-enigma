//==============================================================================
// 
//  Project: mm2hack
//  ScopeGuard.h
// 
//  A pseudo-RAII tech that reproduces finally.
// 
//==============================================================================
#pragma once

#include <functional>
#include <utility>

namespace mm2hack::utils
{
    // The ScopeGuard class is a utility that automatically executes specified cleanup processing when leaving a scope,
    // ensuring that cleanup is performed even if an exception occurs or an early return happens
    class ScopeGuard
    {
    public:
        explicit ScopeGuard(std::function<void()> onExit) 
            : _onExit(std::move(onExit)), _dismissed(false)
        {
        }

        ~ScopeGuard()
        {
            if (!_dismissed) _onExit();
        }

        void Dismiss() { _dismissed = true; }

    private:
        std::function<void()> _onExit;
        bool _dismissed;
    };
}