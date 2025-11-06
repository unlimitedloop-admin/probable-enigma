//==============================================================================
// 
//  Project: mm2hack
//  XInputProvider.h
// 
//  Joypad of XInput provider.
// 
//==============================================================================
#pragma once

#include "IInputProvider.h"

#include <string>
#include "C16ButtonState.h"
#include "KeyBinding.h"

namespace mm2hack::input
{
    // Wrapper for XInput
    class XInputProvider final : public IInputProvider
    {
    public:
        explicit XInputProvider(const KeyBinding& binding)
            : _binding(binding)
        {
        }
        ~XInputProvider() override = default;

        // Update the state of the XInput controller.
        bool Update(C16ButtonState& out_state) override;

    private:
        const std::wstring kClassName = L"XInputProvider";

        const KeyBinding& _binding;     // Key binding for this provider
    };
}