#include "pch.h"

#include "EnemySpawnDirector.h"

#include "apps/runtime/GameContext.h"
#include "apps/world/entity/enemy/EnemyEntity.h"
#include "core/save/StateIO.h"
#include "StageRuntimeContext.h"
#include "utils/output_debug.h"
#include "utils/string_converter.h"

namespace mm2hack::apps::scenes::phases
{
    namespace
    {
        constexpr std::uint32_t kMaximumSlots = 4'096;

        const std::wstring kClassName{ L"EnemySpawnDirector" };

        void log_placement(const world::stage::EnemyPlacement& placement, const std::wstring& message)
        {
            utils::debug_log(kClassName + L": placement \"" + utils::utf8_to_wstring(placement.id) + L"\" " + message);
        }

        [[nodiscard]] bool contains(const foundation::math::RectF& rect, const foundation::math::Vec2& point) noexcept
        {
            return point.x >= rect.left() && point.x < rect.right() &&
                point.y >= rect.top() && point.y < rect.bottom();
        }

        [[nodiscard]] bool overlaps(const foundation::math::RectF& a, const foundation::math::RectF& b) noexcept
        {
            return a.left() < b.right() && b.left() < a.right() &&
                a.top() < b.bottom() && b.top() < a.bottom();
        }
    }

    bool EnemySpawnDirectorState::Save(core::save::StateWriter& writer) const
    {
        if (!IsValid() || !writer.WriteU32(static_cast<std::uint32_t>(slots.size())))
        {
            return false;
        }
        for (const auto& slot : slots)
        {
            if (!writer.WriteU32(slot.tracked_instance_id) ||
                !writer.WriteBool(slot.armed) ||
                !writer.WriteBool(slot.defeated) ||
                !writer.WriteBool(slot.spawned_once))
            {
                return false;
            }
        }
        return true;
    }

    bool EnemySpawnDirectorState::Load(core::save::StateReader& reader)
    {
        std::uint32_t count{};
        if (!reader.ReadU32(count) || count > kMaximumSlots)
        {
            return false;
        }

        EnemySpawnDirectorState loaded{};
        loaded.slots.resize(count);
        for (auto& slot : loaded.slots)
        {
            if (!reader.ReadU32(slot.tracked_instance_id) ||
                !reader.ReadBool(slot.armed) ||
                !reader.ReadBool(slot.defeated) ||
                !reader.ReadBool(slot.spawned_once))
            {
                return false;
            }
        }
        *this = std::move(loaded);
        return true;
    }

    bool EnemySpawnDirectorState::IsValid() const noexcept
    {
        return slots.size() <= kMaximumSlots;
    }

    void EnemySpawnDirector::Build(const StageRuntimeContext& ctx)
    {
        _slots.clear();
        _slots.reserve(ctx.enemy_placements.size());
        const auto& catalog = runtime::GameContext::GetInstance().GetResourceManager().GetEnemyDefinitionCatalog();

        for (const auto& placement : ctx.enemy_placements)
        {
            Slot& slot = _slots.emplace_back();
            slot.placement = placement;

            const int page = ctx.scraper ? ctx.scraper->getPageIndex(static_cast<std::size_t>(placement.room_id)) : -1;
            const auto anchor = (page >= 0 && ctx.page_grid)
                ? ctx.page_grid->ToWorldPosOnPage(page, placement.local_pos)
                : std::nullopt;
            if (!anchor)
            {
                log_placement(placement, L"is in room " + std::to_wstring(placement.room_id) + L", which isn't in the map; ignored.");
                continue;
            }

            slot.definition = catalog.FindById(placement.kind, slot.kind);
            if (slot.definition == nullptr)
            {
                log_placement(placement, L"names kind \"" + utils::utf8_to_wstring(placement.kind) +
                    L"\", which has no loaded definition; ignored.");
                continue;
            }

            slot.palette_index = slot.definition->FindPaletteIndex(placement.palette);
            if (slot.palette_index < 0)
            {
                log_placement(placement, L"asks for palette \"" + utils::utf8_to_wstring(placement.palette) +
                    L"\", which its kind doesn't define; using the default.");
                slot.palette_index = 0;
            }

            slot.anchor_world = *anchor;
            slot.usable = true;
        }
    }

    bool EnemySpawnDirector::mayRespawn_(const Slot& slot) noexcept
    {
        switch (slot.placement.respawn)
        {
        case world::stage::EnemyRespawnPolicy::UntilDefeated: return !slot.state.defeated;
        case world::stage::EnemyRespawnPolicy::Once:          return !slot.state.spawned_once;
        case world::stage::EnemyRespawnPolicy::Always:
        default:                                             return true;
        }
    }

    void EnemySpawnDirector::Tick(StageRuntimeContext& ctx, const Vec2& player_pos)
    {
        using world::entity::enemy::EnemyEntity;

        const auto& view = ctx.scroll->GetView();
        const foundation::math::RectF view_rect{
            view.viewWorldX, view.viewWorldY, static_cast<double>(view.viewW), static_cast<double>(view.viewH) };

        for (auto& slot : _slots)
        {
            if (!slot.usable)
            {
                continue;
            }

            // 1) Follow up on the instance this placement last spawned.
            EnemyEntity* live = nullptr;
            if (slot.state.tracked_instance_id != 0)
            {
                auto* enemy = dynamic_cast<EnemyEntity*>(ctx.entity_mgr->FindByInstanceId(slot.state.tracked_instance_id));
                if (enemy == nullptr || !enemy->IsAlive())
                {
                    // Gone. HP 0 means it was destroyed; anything else (not
                    // held anymore, or killed without damage) was a removal.
                    if (enemy != nullptr && enemy->IsDead())
                    {
                        slot.state.defeated = true;
                    }
                    slot.state.tracked_instance_id = 0;
                }
                else if (slot.placement.despawn_offscreen && !overlaps(enemy->Bounds(), view_rect))
                {
                    enemy->Kill();
                    slot.state.tracked_instance_id = 0;
                }
                else
                {
                    live = enemy;
                }
            }

            // 2) With no live instance, the point leaving the view re-arms it;
            //    the point being in view while armed spawns it.
            const bool point_in_view = contains(view_rect, slot.anchor_world);
            if (live != nullptr)
            {
                continue;
            }
            if (!point_in_view)
            {
                slot.state.armed = true;
                continue;
            }
            if (slot.state.armed && mayRespawn_(slot))
            {
                spawn_(ctx, slot, player_pos);
            }
        }
    }

    void EnemySpawnDirector::spawn_(StageRuntimeContext& ctx, Slot& slot, const Vec2& player_pos)
    {
        using world::entity::enemy::EnemyEntity;

        rendering::sprite::SpriteManager::Id sprite_id{};
        if (!ctx.asset_provider->TryEnemySprite(slot.kind, slot.palette_index, sprite_id))
        {
            log_placement(slot.placement, L"has no loaded sprite for its kind/palette; ignored from now on.");
            slot.usable = false;
            return;
        }

        // Placement points are feet-center; the entity's pos is its sprite's center.
        const Vec2 center{
            slot.anchor_world.x,
            slot.anchor_world.y - slot.definition->abilities.sprite_half_size.y };

        auto& enemy = ctx.entity_mgr->Spawn<EnemyEntity>(
            slot.kind, center, sprite_id, slot.palette_index, slot.definition,
            ctx.asset_provider->EnemyProjectileSprite());
        enemy.SetTerrainProbe(ctx.terrain_probe.get());

        switch (slot.placement.facing)
        {
        case world::stage::PlacementFacing::Left:  enemy.SetFacing(-1); break;
        case world::stage::PlacementFacing::Right: enemy.SetFacing(+1); break;
        case world::stage::PlacementFacing::TowardPlayer:
        default:                                   enemy.SetFacing(player_pos.x < center.x ? -1 : +1); break;
        }

        slot.state.tracked_instance_id = enemy.StateInstanceId();
        slot.state.armed = false;
        slot.state.spawned_once = true;
    }

    EnemySpawnDirectorState EnemySpawnDirector::CaptureState() const
    {
        EnemySpawnDirectorState state{};
        state.slots.reserve(_slots.size());
        for (const auto& slot : _slots)
        {
            state.slots.push_back(slot.state);
        }
        return state;
    }

    bool EnemySpawnDirector::RestoreState(const EnemySpawnDirectorState& state)
    {
        // A save taken with a different .def (placements added/removed)
        // can't be mapped back onto these slots.
        if (!state.IsValid() || state.slots.size() != _slots.size())
        {
            return false;
        }
        for (std::size_t i = 0; i < _slots.size(); ++i)
        {
            _slots[i].state = state.slots[i];
        }
        return true;
    }
}
