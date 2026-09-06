#include "pch.h"

#include "EntityStateFactory.h"

#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/scenes/IStageAssetProvider.h"
#include "apps/world/entity/effects/ChargeEffectEntity.h"
#include "apps/world/entity/effects/ProjectileEntity.h"
#include "apps/world/entity/effects/SlidingDustEffectEntity.h"
#include "apps/world/entity/effects/SplashEffectEntity.h"
#include "core/save/StateIO.h"
#include "EntityManager.h"
#include "IEntity.h"

namespace mm2hack::apps::world::entity
{
    namespace
    {
        std::string ToBinaryString(const std::vector<std::uint8_t>& payload)
        {
            std::string bytes{};
            bytes.reserve(payload.size());
            for (const std::uint8_t value : payload)
            {
                bytes.push_back(static_cast<char>(value));
            }
            return bytes;
        }

        template <typename State, typename Loader>
        bool ParseState(const EntityStateRecord& record, State& state, Loader&& loader)
        {
            std::istringstream payload(ToBinaryString(record.payload), std::ios::in | std::ios::binary);
            core::save::StateReader reader(payload);
            return loader(state, reader) &&
                payload.peek() == std::char_traits<char>::eof();
        }

        bool ParseProjectile(const EntityStateRecord& record, effects::ProjectileEntityState& state)
        {
            if (record.component_version != effects::ProjectileEntity::kStateVersion)
            {
                return false;
            }
            return ParseState(record, state,
                [](effects::ProjectileEntityState& value, core::save::StateReader& reader)
                {
                    return value.Load(reader);
                });
        }

        bool ParseTimedEffect(
            const EntityStateRecord& record,
            std::uint16_t expected_version,
            std::int32_t maximum_ticks,
            TimedEffectEntityState& state)
        {
            if (record.component_version != expected_version)
            {
                return false;
            }
            return ParseState(record, state,
                [maximum_ticks](TimedEffectEntityState& value, core::save::StateReader& reader)
                {
                    return value.Load(reader, maximum_ticks);
                });
        }

        bool IsValidSpriteId(rendering::sprite::SpriteManager::Id id) noexcept
        {
            return id != static_cast<rendering::sprite::SpriteManager::Id>(-1);
        }
    }

    bool EntityStateFactory::ValidateTransientRecord(const EntityStateRecord& record)
    {
        switch (record.type)
        {
        case EntityTypeId::Projectile:
        {
            effects::ProjectileEntityState state{};
            return ParseProjectile(record, state);
        }
        case EntityTypeId::ChargeEffect:
        {
            TimedEffectEntityState state{};
            return ParseTimedEffect(
                record,
                effects::ChargeEffectEntity::kStateVersion,
                effects::ChargeEffectEntity::kTotalTicks,
                state);
        }
        case EntityTypeId::SlidingDustEffect:
        {
            TimedEffectEntityState state{};
            return ParseTimedEffect(
                record,
                effects::SlidingDustEffectEntity::kStateVersion,
                effects::SlidingDustEffectEntity::kTotalTicks,
                state);
        }
        case EntityTypeId::SplashEffect:
        {
            TimedEffectEntityState state{};
            return ParseTimedEffect(
                record,
                effects::SplashEffectEntity::kStateVersion,
                effects::SplashEffectEntity::kTotalTicks,
                state);
        }
        default:
            return false;
        }
    }

    std::unique_ptr<IEntity> EntityStateFactory::CreateTransient(
        const EntityStateRecord& record) const
    {
        switch (record.type)
        {
        case EntityTypeId::Projectile:
        {
            effects::ProjectileEntityState state{};
            const auto sprite_id = _assets.EffectsSprite();
            if (!ParseProjectile(record, state) || !IsValidSpriteId(sprite_id))
            {
                return nullptr;
            }
            return std::make_unique<effects::ProjectileEntity>(state, sprite_id);
        }
        case EntityTypeId::ChargeEffect:
        {
            TimedEffectEntityState state{};
            const auto sprite_id = _assets.ChargeEffectSprite();
            if (!ParseTimedEffect(
                    record,
                    effects::ChargeEffectEntity::kStateVersion,
                    effects::ChargeEffectEntity::kTotalTicks,
                    state) ||
                !IsValidSpriteId(sprite_id))
            {
                return nullptr;
            }
            return std::make_unique<effects::ChargeEffectEntity>(state, sprite_id);
        }
        case EntityTypeId::SlidingDustEffect:
        {
            TimedEffectEntityState state{};
            const auto sprite_id = _assets.SlidingDustEffectSprite();
            if (!ParseTimedEffect(
                    record,
                    effects::SlidingDustEffectEntity::kStateVersion,
                    effects::SlidingDustEffectEntity::kTotalTicks,
                    state) ||
                !IsValidSpriteId(sprite_id))
            {
                return nullptr;
            }
            return std::make_unique<effects::SlidingDustEffectEntity>(state, sprite_id);
        }
        case EntityTypeId::SplashEffect:
        {
            TimedEffectEntityState state{};
            const auto sprite_id = _assets.EffectsSprite();
            if (!ParseTimedEffect(
                    record,
                    effects::SplashEffectEntity::kStateVersion,
                    effects::SplashEffectEntity::kTotalTicks,
                    state) ||
                !IsValidSpriteId(sprite_id))
            {
                return nullptr;
            }
            return std::make_unique<effects::SplashEffectEntity>(state, sprite_id);
        }
        default:
            return nullptr;
        }
    }
}
