//==============================================================================
// 
//  Project: mm2hack
//  FpsManager.h
// 
//  Manage the Fps class.
// 
//==============================================================================
#pragma once

#include "config/SystemConfig.h"
#include "Fps.h"

namespace mm2hack::utils
{
    // Singleton manager for the Fps class to ensure a single instance is used throughout the application
    class FpsManager
    {
    public:
        static Fps& GetInstance()
        {
            static Fps instance(config::SystemConfig::kTargetFps);
            return instance;
        }
    };
}