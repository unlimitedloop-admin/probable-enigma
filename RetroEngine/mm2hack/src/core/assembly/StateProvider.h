//==============================================================================
// 
//  Project: mm2hack
//  StateProvider.h
// 
//  Input state provider abstract class for handling user input.
// 
//==============================================================================
#pragma once

#include <cstdint>
#include <vector>
#include "IInputBackend.h"
#include "InputTypes.h"

namespace mm2hack::core::assembly
{
    // Defines a logical binding that maps a logical key to one or more physical inputs
    struct LogicalBinding
    {
        Key16 key{};
        std::vector<PhysicalBinding> ors; // Arbitrary number of OR connections (ON if any are pressed)
        float analogThreshold{ 0.5f };    // Threshold for analog-to-digital conversion
    };

    // Abstract class for input state providers
    class StateProvider
    {
    public:
        virtual ~StateProvider() = default;

        // Called at the start of each tick. Backend polling -> logical state update
        virtual void BeginTick(std::uint64_t tick) noexcept = 0;
        // Called at the end of each tick. Finalize state for this tick
        virtual void EndTick() noexcept = 0;
        // Indirectly update the joystick state (the actual update is done externally)
        virtual bool UpdateJoystick() noexcept = 0;

        // Get snapshot (value copy)
        [[nodiscard]] virtual InputSnapshot GetSnapshot() const = 0;

        // ---- Single item query ----
        [[nodiscard]] virtual const KeyFrameState& Get(Key16 k) const noexcept = 0;
        [[nodiscard]] virtual bool IsPressed(Key16 k) const noexcept = 0;
        [[nodiscard]] virtual std::int32_t Frames(Key16 k) const noexcept = 0;
        [[nodiscard]] virtual std::int32_t PressedFrames(Key16 k) const noexcept
        {
            const auto f = Frames(k);
            return f > 0 ? f : 0;
        }
        [[nodiscard]] virtual std::int32_t ReleasedFrames(Key16 k) const noexcept
        {
            const auto f = Frames(k);
            return f < 0 ? -f : 0;
        }
        [[nodiscard]] virtual bool JustPressed(Key16 k) const noexcept
        {
            return Frames(k) == 1;
        }
        [[nodiscard]] virtual bool JustReleased(Key16 k) const noexcept
        {
            return Frames(k) == -1;
        }

        // Remapping (can be replaced from UI)
        virtual void SetBindings(const std::vector<LogicalBinding>& map) = 0;
    };
}