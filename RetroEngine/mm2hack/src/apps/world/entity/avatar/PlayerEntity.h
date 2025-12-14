//==============================================================================
// 
//  Project: mm2hack
//  PlayerEntity.h
// 
//  The one player character controlling all actions.
// 
//==============================================================================
#pragma once

#include "apps/systems/physics/ICollider.h"
#include "apps/world/entity/EntityBase.h"

#include <array>
#include <memory>
#include <string>
#include "abilities/ServiceModules.h"
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/systems/physics/CollisionLayer.h"
#include "apps/systems/physics/ITerrainProbe.h"
#include "apps/systems/physics/Probes.h"
#include "apps/systems/physics/TileAttribute.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/world/entity/common/AnimeStepper.h"
#include "apps/world/entity/IEntity.h"
#include "AvatarStatus.h"
#include "IPlayerState.h"
#include "PlayerContext.h"
#include "PlayerParams.h"

namespace mm2hack::core::assembly
{
    class StateProvider;
}

namespace mm2hack::apps::world::entity::avatar
{
    // User player character entity
    class PlayerEntity final : public EntityBase, public systems::physics::ICollider
    {
        using AnimeStepper      = common::AnimeStepper;
        using RectF             = foundation::math::RectF;
        using Vec2              = foundation::math::Vec2;
        using CollisionLayer    = systems::physics::CollisionLayer;
        using ITerrainProbe     = systems::physics::ITerrainProbe;
        using TileAttribute     = systems::physics::TileAttribute;
        using Probes            = systems::physics::Probes;
        using LayerView         = systems::view::Layer;
        using RenderContext     = systems::view::RenderContext;
        using StateProvider     = core::assembly::StateProvider;

        using SpriteManagerId   = rendering::sprite::SpriteManager::Id;

    public:
        PlayerEntity(SpriteManagerId id);

        // Main action updates (IUpdatable)
        void Update(double /*dt*/) override;
        // Drawing layer (IRenderable)
        LayerView DrawLayer() const noexcept override;
        // Rendering (IRenderable)
        void Render(RenderContext& ctx) override;
        // Is on alive? (IEntity)
        bool IsAlive() const noexcept override;
        // Kill (IEntity)
        void Kill() noexcept override;
        // Bounding box (ICollider)
        RectF Bounds() const override;
        // Is collidable? (ICollider)
        bool IsCollidable() const noexcept override;
        // Collision layer (ICollider)
        CollisionLayer Layer() const noexcept override;
        // Collision reactions (ICollider)
        void OnTileCollision(const Vec2& normal, TileAttribute attr) override;
        // Entity collision reaction (ICollider)
        void OnEntityCollision(IEntity& other) override;

        // Return the owner entity
        IEntity& OwnerEntity() noexcept override;
        const IEntity& OwnerEntity() const noexcept override;

        // Set collidable
        void SetCollidable(bool v) noexcept;

        // ===== dependency injection & configuration =====
        void SetInput(StateProvider* in) { _input = in; }
        void SetTuning(const PlayerTuning& t) { _tuning = t; }
        void SetTerrainProbe(ITerrainProbe* p) noexcept { _terrainProbe = p; }
        void SetLadderService(abilities::ILadderService* s) { _ladderService = s; }

        // ===== public parameters =====
        bool onGround{ false };
        AvatarDirection facingLR{ AvatarDirection::Right };
        int  texture{ 0 };

    private:
        void refreshProbes_(PlayerContext& cx) noexcept;

    private:
        const std::wstring kClassName{ L"PlayerEntity" };

        std::unique_ptr<IPlayerState>& FindState(AvatarStatus s)
        {
            switch (s)
            {
            case AvatarStatus::Running:   return _states[1];
            case AvatarStatus::Hovering:  return _states[2];
            case AvatarStatus::LaunchRun: return _states[3];
            case AvatarStatus::BrakeRun:  return _states[4];
            case AvatarStatus::Ladder:    return _states[5];
            case AvatarStatus::Landing:   return _states[6];
            case AvatarStatus::Standing:
            default:                      return _states[0];
            }
        }

        SpriteManagerId _id{};                                          // Sprite Id

        Vec2 _half{};                                                   // Half-size of the bounding box
        bool _collidable{ true };                                       // Whether collision is enabled
        AvatarStatus _status{ AvatarStatus::Standing };                 // Current avatar status
        std::array<std::unique_ptr<IPlayerState>, 7> _states{};

        StateProvider*                 _input{};                        // Player input snapshot (This is separate from core::assembly::InputSnapshot)
        PlayerTuning                   _tuning{};                       // Player tuning parameters
        AnimeStepper                   _animeStepper{};                 // Animation stepper
        Probes                         _probes{ _half };                // Collision probes
        const ITerrainProbe*           _terrainProbe{ nullptr };        // Terrain probe
        abilities::ILadderService*     _ladderService{ nullptr };       // Ladder action service
    };
}