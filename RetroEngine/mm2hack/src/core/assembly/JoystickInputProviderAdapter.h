//==============================================================================
// 
//  Project: mm2hack
//  JoystickInputProviderAdapter.h
// 
//  Joystick input provider adapter for handling joystick input states.
// 
//==============================================================================
#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include "input/JoystickManager.h"
#include "input/Jpbtn.h"
#include "InputTypes.h"
#include "StateProvider.h"

namespace mm2hack::core::assembly
{
    // Adapter class for joystick input provider using abstract class StateProvider
    class JoystickInputProviderAdapter final : public StateProvider
    {
    public:
        explicit JoystickInputProviderAdapter(input::JoystickManager& jm) noexcept : _jm(jm) {/* Undefined */}

        // Called at the start of each tick. Backend polling -> logical state update in KeyFrameState
        void BeginTick(std::uint64_t tick) noexcept override;
        // Called at the end of each tick. Finalize state for this tick
        void EndTick() noexcept override {}
        // Indirectly update the joystick state (the actual update is done externally)
        bool UpdateJoystick() noexcept override { return _jm.Update(); }
        // Get snapshot (value copy), Used for Input-driven replay features etc...
        [[nodiscard]] InputSnapshot GetSnapshot() const override;

        // ---- Single item query ----
        const KeyFrameState& Get(Key16 k) const noexcept override { return _state[static_cast<size_t>(k)]; }
        bool IsPressed(Key16 k) const noexcept override { return Get(k).pressed; }
        bool JustPressed(Key16 k) const noexcept override { auto& s = Get(k); return s.changed && s.pressed; }
        bool JustReleased(Key16 k) const noexcept override { auto& s = Get(k); return s.changed && !s.pressed; }
        std::int32_t Frames(Key16 k) const noexcept { return _state[static_cast<size_t>(k)].frames; }

        void SetBindings(const std::vector<LogicalBinding>&) override
        {
            // Button key remapping is delegated to JoystickManager(NOP).
        }

    private:
        const std::wstring kClassName{ L"JoystickInputProviderAdapter" };

        input::JoystickManager& _jm;    // External joystick manager reference
        std::array<KeyFrameState, static_cast<size_t>(Key16::JPBTN_COUNT)> _state{};    // Current state of all buttons
        std::uint64_t _tick{ 0 };   // Current simulation tick number
    };
}