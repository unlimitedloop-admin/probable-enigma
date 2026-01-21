#include "pch.h"

#include "EntityManager.h"

#include "apps/systems/view/RenderContext.h"
#include "IEntity.h"

namespace mm2hack::apps::world::entity
{
    void EntityManager::Add(std::unique_ptr<IEntity> entity)
    {
        if (!entity)
        {
            return;
        }

        if (_is_updating)
        {
            _pending_add.emplace_back(std::move(entity));
            return;
        }

        _entities.emplace_back(std::move(entity));
    }

    void EntityManager::UpdateAll(double dt)
    {
        _is_updating = true;

        for (auto& e : _entities)
        {
            if (!e || !e->IsAlive())
            {
                continue;
            }

            e->Update(dt);
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
    }

    std::size_t EntityManager::Count() const noexcept
    {
        return _entities.size() + _pending_add.size();
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