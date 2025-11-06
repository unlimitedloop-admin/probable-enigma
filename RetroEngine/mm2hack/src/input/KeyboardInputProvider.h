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
#include <string>
#include "C16ButtonState.h"
#include "KeyBinding.h"
#include "KeyboardsIn.h"

namespace mm2hack::input
{
    // Keyboard input provider, used when no Joycard is connected
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
        const std::wstring kClassName = L"KeyboardInputProvider";

        std::unique_ptr<KeyboardsIn> _keyboard; // Keyboard input manager with composition relationship
        const KeyBinding& _binding;             // Key binding for this provider
    };
}