//==============================================================================
// 
//  Project: mm2hack
//  IInputProvider.h
// 
//  Update the input state interface.
// 
//==============================================================================
#pragma once

#include "C16ButtonState.h"

namespace mm2hack::input
{
    // Interface for input providers
    class IInputProvider
    {
    public:
        virtual ~IInputProvider() = default;
        // Update the input state and return the updated state
        virtual bool Update(C16ButtonState& out_state) = 0;
    };
}