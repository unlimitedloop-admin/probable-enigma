//==============================================================================
// 
//  Project: mm2hack
//  DirectInputProvider.h
// 
//  Joypad of DirectInput provider.
// 
//==============================================================================
#pragma once

#include "IInputProvider.h"

#include <string>
#include "C16ButtonState.h"
#include "KeyBinding.h"

namespace mm2hack::input
{
    // Wrapper for DirectInput (DirectInputDevice 8.0)
    class DirectInputProvider final : public IInputProvider
    {
    public:
        explicit DirectInputProvider(const KeyBinding& binding)
            : _binding(binding)
        {
        }
        ~DirectInputProvider() override = default;

        // Update the state of the DirectInput controller.
        bool Update(C16ButtonState& out_state) override;

    private:
        const std::wstring kClassName{ L"DirectInputProvider" };

        const KeyBinding& _binding;     // Key binding for this provider
    };
}