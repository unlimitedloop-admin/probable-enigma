//==============================================================================
//
//  Project: mm2hack
//  EnemySpawnDirector.h
//
//  Brings a stage's enemy placements (from its .def) to life: spawns a
//  placed enemy when its placement point scrolls into view, removes it
//  again once it leaves the view, and decides -- per the placement's
//  respawn policy -- whether it may come back the next time the point
//  scrolls in. The classic rule: a point has to leave the view (with no
//  live instance of it around) before it can spawn again.
//
//==============================================================================
#pragma once

#include <cstdint>
#include <vector>

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/world/entity/enemy/animation/AnimationTypes.h"
#include "apps/world/entity/enemy/lists/EnemyLists.h"
#include "apps/world/entity/IEntity.h"
#include "apps/world/stage/StagePlacement.h"
#include "core/save/StateIO.h"

namespace mm2hack::apps::scenes::phases
{
    struct StageRuntimeContext;

    // Per-placement bookkeeping; the only part of the director that changes
    // at runtime, so the only part saved.
    struct EnemySpawnSlotState final
    {
        world::entity::EntityInstanceId tracked_instance_id{ 0 }; // Live instance, 0 = none
        bool armed{ true };         // May spawn the next time its point is in view
        bool defeated{ false };     // An instance of it was destroyed (HP 0) this stage
        bool spawned_once{ false };
    };

    struct EnemySpawnDirectorState final
    {
        std::vector<EnemySpawnSlotState> slots;

        bool Save(core::save::StateWriter& writer) const;
        bool Load(core::save::StateReader& reader);
        [[nodiscard]] bool IsValid() const noexcept;
    };

    class EnemySpawnDirector final
    {
        using Vec2 = foundation::math::Vec2;

    public:
        // Resolves every placement in ctx.enemy_placements against the loaded
        // map and enemy definitions. One that can't be resolved (unknown room
        // or kind) is logged and kept as an inert slot, so slot indices --
        // and saved state -- always line up with the .def's order.
        void Build(const StageRuntimeContext& ctx);
        // Run once per tick AFTER the entity collision pass: anything that
        // died in it is still held by the EntityManager (it's only removed
        // during the next UpdateAll()), so a kill can be told from a despawn.
        void Tick(StageRuntimeContext& ctx, const Vec2& player_pos);

        [[nodiscard]] EnemySpawnDirectorState CaptureState() const;
        bool RestoreState(const EnemySpawnDirectorState& state);

    private:
        struct Slot final
        {
            world::stage::EnemyPlacement placement{};
            bool usable{ false };
            Vec2 anchor_world{};        // Feet-center, in world pixels
            world::entity::enemy::EnemyKind kind{};
            const world::entity::enemy::animation::EnemyDefinition* definition{ nullptr };
            int palette_index{ 0 };
            EnemySpawnSlotState state{};
        };

        [[nodiscard]] static bool mayRespawn_(const Slot& slot) noexcept;
        void spawn_(StageRuntimeContext& ctx, Slot& slot, const Vec2& player_pos);

        std::vector<Slot> _slots;
    };
}
