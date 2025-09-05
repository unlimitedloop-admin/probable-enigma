//==============================================================================
// 
//  Project: mm2hack
//  InputLoadResult.h
// 
//  Configuration result for input device loading.
// 
//==============================================================================
#pragma once

#include "input/KeyToken.h"

namespace mm2hack::config
{
    // Result of loading input device configuration
    struct InputLoadResult
    {
        bool applied;
        input::Device savedProvider;
    };
}