//==============================================================================
// 
//  Project: mm2hack
//  InputConfigOverlay.h
// 
//  Join the input configuration mode overlay.
// 
//==============================================================================
#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <vector>
#include "input/Jpbtn.h"
#include "input/KeyBinding.h"
#include "input/KeyToken.h"
#include "input/RawInputEvent.h"

namespace mm2hack::core::overlay
{
    enum class CaptureState : uint8_t { Hidden, Intro, Quiescent, Listening, Confirm, Completed, Cancelled };

    // A step in the input configuration process
    struct CaptureStep
    {
        JPBTN jpbtn;
        const char* label;
    };

    inline std::vector<CaptureStep> BuildStepsFull16()
    {
        // The key binding procedure will proceed in the order written here.
        return {
            { JPBTN::UP,          "Up" },
            { JPBTN::DOWN,        "Down" },
            { JPBTN::LEFT,        "Left" },
            { JPBTN::RIGHT,       "Right" },
            { JPBTN::A,           "A" },
            { JPBTN::B,           "B" },
            { JPBTN::X,           "X" },
            { JPBTN::Y,           "Y" },
            { JPBTN::START,       "Start" },
            { JPBTN::BACK,        "Back (Select)" },
            { JPBTN::LSHOULDER,   "LB (L1)" },
            { JPBTN::RSHOULDER,   "RB (R1)" },
            { JPBTN::LTRIGGER,    "LT (L2)" },
            { JPBTN::RTRIGGER,    "RT (R2)" },
            { JPBTN::LTHUMB,      "LStick (L3)" },
            { JPBTN::RTHUMB,      "RStick (R3)" },
        };
    }

    // Overlay for configuring input bindings
    class InputConfigOverlay final
    {
    public:
        static InputConfigOverlay& GetInstance()
        {
            static InputConfigOverlay instance;
            return instance;
        }

        InputConfigOverlay(const InputConfigOverlay&) = delete;
        InputConfigOverlay& operator=(const InputConfigOverlay&) = delete;
        InputConfigOverlay(InputConfigOverlay&&) = delete;
        InputConfigOverlay& operator=(InputConfigOverlay&&) = delete;

        // Begin input configuration for the specified KeyBinding and steps
        void Open(input::KeyBinding& target, std::vector<CaptureStep> steps);
        // Cancel the input configuration
        void Cancel() noexcept;
        // Check if the overlay is currently open
        bool IsOpen() const noexcept;
        // Controlling the overlay (should be called every frame)
        void Tick(float dtSec);

    private:
        InputConfigOverlay() = default;
        ~InputConfigOverlay() = default;

        // Rendering and internal logic
        void render() const;
        void advance();
        void adoptBinding(const input::RawInputEvent& e);
        void onStepEntered();

        input::KeyBinding* _target{ nullptr };
        std::vector<CaptureStep> _steps;
        size_t _index{ 0 };
        CaptureState _state{ CaptureState::Hidden };
        input::Device _captureKind{ input::Device::Keyboard };

        std::optional<input::RawInputEvent> _candidate;
        std::chrono::steady_clock::time_point _stateStart{};
        int _requiredQuiescentMs{ 150 };   // Quiescent time before accepting input
        float _analogThreshold{ 0.5f };    // Analog input threshold
        bool _allowMultiple{ false };      // Allow multiple bindings

        float _confirmElapsedSec{ 0.0f };
        float _confirmQuietSec{ 0.0f };
        int _confirmMinShowMs{ 250 };
        int _confirmQuietMs{ 150 };
    };
}