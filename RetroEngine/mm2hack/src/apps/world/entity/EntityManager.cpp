#include "pch.h"

#include "EntityManager.h"

#include <limits>
#include "apps/systems/view/RenderContext.h"
#include "apps/systems/view/ViewState.h"
#include "IEntity.h"

namespace mm2hack::apps::world::entity
{
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
