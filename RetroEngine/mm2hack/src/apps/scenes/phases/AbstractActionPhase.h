//==============================================================================
// 
//  Project: mm2hack
//  AbstractActionPhase.h
// 
//  Abstract action stage module that manages runtime context and stage script.
// 
//==============================================================================
#pragma once

#include "IPhase.h"

#include <memory>
#include <string>
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/ui/productions/StageIntroUI.h"
#include "IPhaseHost.h"
#include "PhaseResult.h"
#include "StageRuntimeContext.h"

namespace mm2hack::apps::resources::parameters
{
    class Parameters;
}

namespace mm2hack::apps::world::entity::avatar
{
    class PlayerEntity;
}

namespace mm2hack::apps::scenes::phases
{
    class IStageScript;

    // States within the action phase
    enum class ActionPhaseState
    {
        Intro,  // avatar character warp animation, and more
        Active  // main gameplay state
    };

    // Abstract action phase that manages the runtime context and stage script
    class AbstractActionPhase final : public IPhase
    {
        using Vec2 = foundation::math::Vec2;

    public:
        AbstractActionPhase(std::unique_ptr<StageRuntimeContext> ctx, IStageScript* script, IPhaseHost& host) noexcept;
        ~AbstractActionPhase() override;

        AbstractActionPhase(const AbstractActionPhase&) = delete;
        AbstractActionPhase& operator=(const AbstractActionPhase&) = delete;

        // Sets up the phase with given parameters
        void Initialize(const resources::parameters::Parameters& params) override;
        // Updates the phase and returns the result
        PhaseResult Update() override;
        // Renders the world elements
        void RenderWorld() override;
        // Renders the overlay elements
        void RenderOverlay() override;
        // Enables or disables the operate phase
        void SetEnableOperatePhase(bool enable) override;
        // Gets whether the operate phase is enabled
        bool GetEnableOperatePhase() const override { return _operate; }

    private:
        void updateIntro_();                                // Handles the intro state update
        void updateActive_();                               // Handles the active state update
        void consumePlayerOutput_(world::entity::avatar::PlayerEntity& player); // Handles player events and spawn commands

    private:
        const std::wstring kClassName{ L"AbstractActionPhase" };

        enum class IntroStep
        {
            Standby,
            ReadyBlink,
            WarpIn,
            Done
        };

        struct IntroSequence
        {
            IntroStep step{ IntroStep::Standby };
            double    timer{ 0.0 };
        } _intro{};                                         // Intro sequence state

        std::unique_ptr<StageRuntimeContext> _ctx{};        // Runtime context for the stage
        std::wstring _bgm_key{};                            // Current BGM key
        IStageScript* _script{};                            // Optional stage script for custom behavior
        IPhaseHost* _host{};                                // Host for phase transitions
        bool _entered{ false };                             // Indicates if the phase has been entered
        bool _operate{ false };                             // Indicates if the operate phase is enabled (Disable at fade-in and fade-out)

        Vec2 _player_prev_pos{};                            // Previous player position, scrolling-player sync use
        ActionPhaseState _state{ ActionPhaseState::Intro }; // Current state of the action phase
        ui::productions::StageIntroUI _ready_ui{};          // UI for the intro sequence

        // ======== debug info ========
        int _page_index_debug{ 0 };
        double _player_pos_x_debug{ 0 };
        double _player_pos_y_debug{ 0 };
    };
}
