//==============================================================================
// 
//  Project: mm2hack
//  EntityManager.h
// 
//  Manages sprite data that is active in the current scene.
// 
//==============================================================================
#pragma once

#include <memory>
#include <string>
#include <type_traits>
#include <vector>
#include "apps/systems/view/RenderContext.h"
#include "IEntity.h"

namespace mm2hack::apps::world::entity
{
    // Manages lifetime/update/render for entities
    // This is a minimal version to support spawning projectiles/effects etc
    class EntityManager final
    {
    public:
        EntityManager() = default;
        EntityManager(const EntityManager&) = delete;
        EntityManager& operator=(const EntityManager&) = delete;
        EntityManager(EntityManager&&) = default;
        EntityManager& operator=(EntityManager&&) = default;
        ~EntityManager() = default;

        // Adds an entity to the manager
        // If called during UpdateAll, the entity will be deferred until the end of the update
        void Add(std::unique_ptr<IEntity> entity);

        // Spawns an entity of type T and returns reference.
        template <typename T, typename... Args>
        T& Spawn(Args&&... args);

        // Updates all alive entities, flushes pending entities, and removes dead entities
        void UpdateAll(double dt);

        // Renders entities for the specified layer
        void RenderLayer(systems::view::RenderContext& ctx, systems::view::Layer layer);

        // Removes all entities immediately.
        void Clear() noexcept;

        // Finds the first entity of type T (dynamic_cast). Returns nullptr if not found
        template <typename T>
        T* FindFirst() noexcept;
        template <typename T>
        const T* FindFirst() const noexcept;

        // Counts the number of alive entities of type T (dynamic_cast)
        template <typename T>
        std::size_t CountAlive() const noexcept;

        std::size_t Count() const noexcept;

    private:
        void flushPending_();
        void removeDead_();

    private:
        const std::wstring kClassName{ L"EntityManager" };

        std::vector<std::unique_ptr<IEntity>> _entities{};
        std::vector<std::unique_ptr<IEntity>> _pending_add{};
        bool _is_updating{ false };
    };

    template <typename T, typename... Args>
    T& EntityManager::Spawn(Args&&... args)
    {
        static_assert(std::is_base_of_v<IEntity, T>, "T must derive from IEntity.");

        auto entity = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *entity;
        Add(std::move(entity));
        return ref;
    }

    template <typename T>
    T* EntityManager::FindFirst() noexcept
    {
        static_assert(std::is_base_of_v<IEntity, T>, "T must derive from IEntity.");

        for (auto& e : _entities)
        {
            if (!e || !e->IsAlive())
            {
                continue;
            }

            if (auto* p = dynamic_cast<T*>(e.get()); p != nullptr)
            {
                return p;
            }
        }
        return nullptr;
    }

    template <typename T>
    const T* EntityManager::FindFirst() const noexcept
    {
        static_assert(std::is_base_of_v<IEntity, T>, "T must derive from IEntity.");

        for (const auto& e : _entities)
        {
            if (!e || !e->IsAlive())
            {
                continue;
            }

            if (const auto* p = dynamic_cast<const T*>(e.get()); p != nullptr)
            {
                return p;
            }
        }
        return nullptr;
    }

    template <typename T>
    std::size_t EntityManager::CountAlive() const noexcept
    {
        static_assert(std::is_base_of_v<IEntity, T>, "T must derive from IEntity.");

        std::size_t count = 0;
        for (const auto& e : _entities)
        {
            if (!e || !e->IsAlive()) continue;

            if (dynamic_cast<const T*>(e.get()) != nullptr)
            {
                ++count;
            }
        }

        return count;
    }
}