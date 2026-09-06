//==============================================================================
// 
//  Project: mm2hack
//  EntityBase.h
// 
//  Base class for all entities in the game.
// 
//==============================================================================
#pragma once

#include "IEntity.h"

#include <cstdint>
#include "apps/foundation/math/CoordinateTypes.h"
#include "core/save/StateIO.h"

namespace mm2hack::apps::world::entity
{
    using foundation::math::Vec2;

    struct EntityKinematicState final
    {
        Vec2 position{};
        Vec2 velocity{};

        bool Save(core::save::StateWriter& writer) const;
        bool Load(core::save::StateReader& reader);
        [[nodiscard]] bool IsValid() const noexcept;
    };

    struct TimedEffectEntityState final
    {
        EntityKinematicState kinematic{};
        std::int32_t base_texture{};
        std::int32_t elapsed_ticks{};

        bool Save(core::save::StateWriter& writer, std::int32_t maximum_ticks) const;
        bool Load(core::save::StateReader& reader, std::int32_t maximum_ticks);
        [[nodiscard]] bool IsValid(std::int32_t maximum_ticks) const noexcept;
    };

    // Basic implementation of IEntity
    class EntityBase : public IEntity
    {
    public:
        bool IsAlive() const noexcept override { return _alive; }
        void Kill() noexcept override { _alive = false; }
        [[nodiscard]] EntityInstanceId StateInstanceId() const noexcept override { return _instance_id; }
        void AssignStateInstanceId(EntityInstanceId id) noexcept override { _instance_id = id; }
        [[nodiscard]] EntityKinematicState CaptureKinematicState() const noexcept;
        bool RestoreKinematicState(const EntityKinematicState& state) noexcept;

        // Position and velocity
        Vec2 pos{};
        Vec2 vel{};

    protected:
        bool _alive{ true };

    private:
        EntityInstanceId _instance_id{};
    };
}
