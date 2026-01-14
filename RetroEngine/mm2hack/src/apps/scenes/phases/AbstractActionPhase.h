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

        void Initialize(const resources::parameters::Parameters& params) override;
        PhaseResult Update() override;

        void RenderWorld() override;
        void RenderOverlay() override;

    private:
        std::unique_ptr<StageRuntimeContext> _ctx{};
        IStageScript* _script{};
        IPhaseHost* _host{};
        bool _entered{ false };

        Vec2 _player_prev_pos{};                                // Previous player position, scrolling-player sync use

        // debug info.
        int _page_index_debug{ 0 };
        double _player_pos_x_debug{ 0 };
        double _player_pos_y_debug{ 0 };
    };
}