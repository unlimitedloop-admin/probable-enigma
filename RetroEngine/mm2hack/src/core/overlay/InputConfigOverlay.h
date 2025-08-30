//==============================================================================
// 
//  Project: mm2hack
//  ***.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <vector>
#include "input/Jpbtn.h"
#include "input/KeyBinding.h"
#include "input/RawInputEvent.h"

namespace mm2hack::core::overlay
{

    enum class CaptureState : uint8_t { Hidden, Intro, Quiescent, Listening, Confirm, Completed, Cancelled };

    struct CaptureStep
    {
        JPBTN jpbtn;        // Up, Down, A, B, ...
        const char* label;  // "Up" 等
    };

    inline std::vector<CaptureStep> BuildStepsFull16()
    {
        using mm2hack::JPBTN;
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
            { JPBTN::BACK,        "Back" },
            { JPBTN::LSHOULDER,   "LB" },
            { JPBTN::RSHOULDER,   "RB" },
            { JPBTN::LTRIGGER,    "LT" },
            { JPBTN::RTRIGGER,    "RT" },
            { JPBTN::LTHUMB,      "LStick Click" },
            { JPBTN::RTHUMB,      "RStick Click" },
        };
    }

    class InputConfigOverlay final
    {
    public:
        static InputConfigOverlay& GetInstance()
        {
            static InputConfigOverlay instance;
            return instance;
        }

        static uint16_t ToToken(const input::RawInputEvent& e, uint8_t thrNibbleDefault = 8);

        void Open(input::KeyBinding& target, std::vector<CaptureStep> steps);
        void Cancel() noexcept;
        bool IsOpen() const noexcept;

        // 毎フレーム呼ぶ：RenderOverlay() 内で OK
        void Tick(float dtSec);

    private:
        void render() const;
        void advance();
        void adoptBinding(const input::RawInputEvent& e);

        input::KeyBinding* _target{ nullptr };
        std::vector<CaptureStep> _steps;
        size_t _index{ 0 };
        CaptureState _state{ CaptureState::Hidden };

        std::optional<input::RawInputEvent> _candidate;
        std::chrono::steady_clock::time_point _stateStart{};
        int _requiredQuiescentMs{ 150 };   // 静穏期間
        float _analogThreshold{ 0.5f };    // 軸・トリガの採用しきい値
        bool _allowMultiple{ false };      // 多重許可ポリシー
    };

}