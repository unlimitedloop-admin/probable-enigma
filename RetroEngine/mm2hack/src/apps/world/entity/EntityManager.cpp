#include "pch.h"

#include "EntityManager.h"

#include <cstdint>
#include <limits>
#include <span>
#include <sstream>
#include <string>
#include <utility>
#include "apps/systems/view/RenderContext.h"
#include "apps/systems/view/ViewState.h"
#include "core/save/StateIO.h"
#include "IEntity.h"

namespace mm2hack::apps::world::entity
{
    namespace
    {
        constexpr std::uint32_t kMaximumEntityCount = 1'024;
        constexpr std::uint32_t kMaximumEntityPayloadBytes = 64 * 1'024;
        constexpr std::size_t kMaximumAggregatePayloadBytes = 4 * 1'024 * 1'024;

        bool IsKnownEntityType(EntityTypeId type) noexcept
        {
            return type >= EntityTypeId::Player && type <= EntityTypeId::SplashEffect;
        }
    }

    bool EntityManagerState::Save(core::save::StateWriter& writer) const
    {
        if (!IsValid() ||
            !writer.WriteU32(next_instance_id) ||
            !writer.WriteU32(static_cast<std::uint32_t>(records.size())))
        {
            return false;
        }

        for (const auto& record : records)
        {
            if (!writer.WriteU16(static_cast<std::uint16_t>(record.type)) ||
                !writer.WriteU32(record.instance_id) ||
                !writer.WriteU16(record.component_version) ||
                !writer.WriteU32(static_cast<std::uint32_t>(record.payload.size())) ||
                !writer.WriteBytes(std::span<const std::uint8_t>(record.payload)))
            {
                return false;
            }
        }
        return true;
    }

    bool EntityManagerState::Load(core::save::StateReader& reader)
    {
        EntityManagerState loaded{};
        std::uint32_t count{};
        if (!reader.ReadU32(loaded.next_instance_id) ||
            !reader.ReadU32(count) || count > kMaximumEntityCount)
        {
            return false;
        }

        loaded.records.reserve(count);
        std::size_t aggregate_payload_size{};
        for (std::uint32_t i = 0; i < count; ++i)
        {
            std::uint16_t encoded_type{};
            std::uint32_t payload_size{};
            EntityStateRecord record{};
            if (!reader.ReadU16(encoded_type) ||
                !reader.ReadU32(record.instance_id) ||
                !reader.ReadU16(record.component_version) ||
                !reader.ReadU32(payload_size) ||
                payload_size > kMaximumEntityPayloadBytes ||
                aggregate_payload_size > kMaximumAggregatePayloadBytes - payload_size)
            {
                return false;
            }

            record.type = static_cast<EntityTypeId>(encoded_type);
            record.payload.resize(payload_size);
            if (!reader.ReadBytes(std::span<std::uint8_t>(record.payload)))
            {
                return false;
            }
            aggregate_payload_size += payload_size;
            loaded.records.emplace_back(std::move(record));
        }

        if (!loaded.IsValid())
        {
            return false;
        }
        *this = std::move(loaded);
        return true;
    }

    bool EntityManagerState::IsValid() const noexcept
    {
        if (next_instance_id == 0 || records.size() > kMaximumEntityCount)
        {
            return false;
        }

        std::size_t aggregate_payload_size{};
        for (std::size_t i = 0; i < records.size(); ++i)
        {
            const auto& record = records[i];
            if (!IsKnownEntityType(record.type) ||
                record.instance_id == 0 || record.instance_id >= next_instance_id ||
                record.component_version == 0 ||
                record.payload.size() > kMaximumEntityPayloadBytes ||
                aggregate_payload_size > kMaximumAggregatePayloadBytes - record.payload.size())
            {
                return false;
            }

            for (std::size_t previous = 0; previous < i; ++previous)
            {
                if (records[previous].instance_id == record.instance_id)
                {
                    return false;
                }
            }
            aggregate_payload_size += record.payload.size();
        }
        return true;
    }

    void EntityManager::Add(std::unique_ptr<IEntity> entity)
    {
        if (!entity)
        {
            return;
        }

        if (_next_instance_id == 0)
        {
            return;
        }
        entity->AssignStateInstanceId(_next_instance_id++);

        if (_is_updating)
        {
            _pending_add.emplace_back(std::move(entity));
            return;
        }

        _entities.emplace_back(std::move(entity));
    }

    bool EntityManager::AddRestored(
        std::unique_ptr<IEntity> entity,
        EntityInstanceId instance_id)
    {
        if (!entity || instance_id == 0 ||
            instance_id == std::numeric_limits<EntityInstanceId>::max() ||
            _is_updating || !_pending_add.empty())
        {
            return false;
        }

        for (const auto& current : _entities)
        {
            if (current && current->StateInstanceId() == instance_id)
            {
                return false;
            }
        }

        entity->AssignStateInstanceId(instance_id);
        _entities.emplace_back(std::move(entity));
        if (instance_id >= _next_instance_id)
        {
            _next_instance_id = instance_id + 1;
        }
        return true;
    }

    void EntityManager::UpdateAll(const systems::view::ViewState* view, double dt)
    {
        _is_updating = true;

        for (auto& e : _entities)
        {
            if (!e || !e->IsAlive())
            {
                continue;
            }

            e->Update(view, dt);
        }

        _is_updating = false;

        flushPending_();
        removeDead_();
    }

    void EntityManager::RenderLayer(systems::view::RenderContext& ctx, systems::view::Layer layer)
    {
        ctx.layer = layer;

        for (auto& e : _entities)
        {
            if (!e || !e->IsAlive())
            {
                continue;
            }

            if (e->DrawLayer() != layer)
            {
                continue;
            }

            e->Render(ctx);
        }
    }

    void EntityManager::Clear() noexcept
    {
        _pending_add.clear();
        _entities.clear();
        _is_updating = false;
        _next_instance_id = 1;
    }

    std::size_t EntityManager::Count() const noexcept
    {
        return _entities.size() + _pending_add.size();
    }

    bool EntityManager::CanCaptureState() const noexcept
    {
        return !_is_updating && _pending_add.empty();
    }

    bool EntityManager::CaptureState(EntityManagerState& state) const
    {
        if (!CanCaptureState())
        {
            return false;
        }

        EntityManagerState captured{};
        captured.next_instance_id = _next_instance_id;
        captured.records.reserve(_entities.size());
        for (const auto& entity : _entities)
        {
            if (!entity || !entity->IsAlive())
            {
                continue;
            }

            const auto version = entity->StateComponentVersion();
            if (version == 0)
            {
                return false;
            }

            std::ostringstream payload(std::ios::out | std::ios::binary);
            core::save::StateWriter writer(payload);
            if (!entity->SaveState(writer))
            {
                return false;
            }

            EntityStateRecord record{
                .type = entity->StateTypeId(),
                .instance_id = entity->StateInstanceId(),
                .component_version = version,
            };
            const std::string payload_bytes = payload.str();
            record.payload.reserve(payload_bytes.size());
            for (const char value : payload_bytes)
            {
                record.payload.emplace_back(static_cast<std::uint8_t>(value));
            }
            captured.records.emplace_back(std::move(record));
        }

        if (!captured.IsValid())
        {
            return false;
        }
        state = std::move(captured);
        return true;
    }

    bool EntityManager::RestoreNextInstanceId(EntityInstanceId next_instance_id) noexcept
    {
        if (_is_updating || !_pending_add.empty() ||
            next_instance_id == 0 || next_instance_id < _next_instance_id)
        {
            return false;
        }
        _next_instance_id = next_instance_id;
        return true;
    }

    void EntityManager::flushPending_()
    {
        if (_pending_add.empty())
        {
            return;
        }

        for (auto& e : _pending_add)
        {
            _entities.emplace_back(std::move(e));
        }
        _pending_add.clear();
    }

    void EntityManager::removeDead_()
    {
        std::erase_if(_entities, [](const std::unique_ptr<IEntity>& e)
            {
                return (!e) || (!e->IsAlive());
            });
    }
}
