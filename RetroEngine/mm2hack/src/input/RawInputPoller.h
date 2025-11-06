//==============================================================================
// 
//  Project: mm2hack
//  RawInputPoller.h
// 
//  Polling raw input events.
// 
//==============================================================================
#pragma once

#include <optional>
#include <string>
#include "JoystickManager.h"
#include "KeyToken.h"
#include "RawInputEvent.h"

namespace mm2hack::input
{
    // Polls raw input events from various devices
    class RawInputPoller final
    {
    public:
        RawInputPoller() = delete;
        ~RawInputPoller() = delete;
        RawInputPoller(const RawInputPoller&) = delete;
        RawInputPoller& operator=(const RawInputPoller&) = delete;
        RawInputPoller(RawInputPoller&&) = delete;
        RawInputPoller& operator=(RawInputPoller&&) = delete;
        // This struct is not copyable or movable (static member defined only)

        // Poll the first raw input change event, with optional deadzone and device filter
        static std::optional<RawInputEvent> PollFirstRawChange(
            float deadzone,
            std::optional<Device> only,
            Device activeKind,
            AxisGroup diCaptureGroup
        );

    private:
        // Device-specific polling methods
        static std::optional<RawInputEvent> pollKeyboardRawChange_();                                               // Poll keyboard for key presses
        static std::optional<RawInputEvent> pollXInputRawChange_(float deadzone);                                   // Poll XInput device for button/axis changes
        static std::optional<RawInputEvent> pollDirectInputRawChange_(float deadzone, AxisGroup diCaptureGroup);    // Poll DirectInput device for button/axis changes

    private:
        const std::wstring kClassName = L"RawInputPoller";
    };
}