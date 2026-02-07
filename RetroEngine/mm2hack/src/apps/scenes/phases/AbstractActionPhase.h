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
#include "IPhaseHost.h"
#include "PhaseResult.h"
#include "StageRuntimeContext.h"

namespace mm2hack::apps::resources::parameters
{
    class Parameters;
}

namespace mm2hack::apps::scenes::phases
{
    class IStageScript;

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

    private:
        const std::wstring kClassName{ L"AbstractActionPhase" };

        std::unique_ptr<StageRuntimeContext> _ctx{};    // Runtime context for the stage
        std::wstring _bgm_key{};                        // Current BGM key
        IStageScript* _script{};                        // Optional stage script for custom behavior
        IPhaseHost* _host{};                            // Host for phase transitions
        bool _entered{ false };                         // Indicates if the phase has been entered

        Vec2 _player_prev_pos{};                        // Previous player position, scrolling-player sync use

        // ======== debug info ========
        int _page_index_debug{ 0 };
        double _player_pos_x_debug{ 0 };
        double _player_pos_y_debug{ 0 };
    };
}