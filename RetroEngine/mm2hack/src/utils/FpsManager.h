//==============================================================================
// 
//  Project: mm2hack
//  FpsManager.h
// 
//  Manage the Fps class.
// 
//==============================================================================
#pragma once

#include <string>
#include "Fps.h"

namespace mm2hack::utils
{
    // Singleton manager for the Fps class to ensure a single instance is used throughout the application
    class FpsManager
    {
    public:
        static Fps& GetInstance()
        {
            static Fps instance;
            return instance;
        }

        FpsManager(const FpsManager&) = delete;
        FpsManager& operator=(const FpsManager&) = delete;
        FpsManager(FpsManager&&) = delete;
        FpsManager& operator=(FpsManager&&) = delete;
        // FpsManager is a singleton, so we delete the copy and move constructors and assignment operators.

    private:
        FpsManager() = default;
        ~FpsManager() = default;

    private:
        const std::wstring kClassName = L"FpsManager";
    };
}