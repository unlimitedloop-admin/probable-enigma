//==============================================================================
// 
//  Project: mm2hack
//  KeyboardInputProvider.h
// 
//  Keyboard input provider.
// 
//==============================================================================
#pragma once

#include "IInputProvider.h"

#include <memory>
#include "C16ButtonState.h"
#include "KeyBinding.h"
#include "KeyboardsIn.h"

namespace mm2hack::input
{
    // Keyboard input provider
    class KeyboardInputProvider final : public IInputProvider
    {
    public:
        // Constructor with default binding.
        KeyboardInputProvider(const KeyBinding& binding)
            : _keyboard(std::make_unique<KeyboardsIn>())
            , _binding(binding)
        {
        }

        // Update the input state and return the updated state.
        bool Update(C16ButtonState& out_state) override;

    private:
        std::unique_ptr<KeyboardsIn> _keyboard;
        const KeyBinding& _binding;
    };
}