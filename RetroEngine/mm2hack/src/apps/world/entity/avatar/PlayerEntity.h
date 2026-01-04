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
#include <optional>
#include <string>
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/systems/physics/CollisionLayer.h"
#include "apps/systems/physics/ILadderService.h"
#include "apps/systems/physics/ITerrainProbe.h"
#include "apps/systems/physics/PageGridIndex.h"
#include "apps/systems/physics/Probes.h"
#include "apps/systems/physics/TileAttribute.h"
#include "apps/systems/scrolling/atomic/IScrollRuleProvider.h"
#include "apps/systems/scrolling/atomic/ScrollController.h"
#include "apps/systems/scrolling/atomic/ScrollTypes.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/world/entity/common/AnimeStepper.h"
#include "apps/world/entity/IAnimTickable.h"
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
    class PlayerEntity final : public EntityBase, public systems::physics::ICollider, public IAnimTickable
    {
        using AnimeStepper        = common::AnimeStepper;
        using RectF               = foundation::math::RectF;
        using Vec2                = foundation::math::Vec2;
        using CollisionLayer      = systems::physics::CollisionLayer;
        using ILadderService      = systems::physics::ILadderService;
        using ITerrainProbe       = systems::physics::ITerrainProbe;
        using TileAttribute       = systems::physics::TileAttribute;
        using PageGridIndex       = systems::physics::PageGridIndex;
        using Probes              = systems::physics::Probes;
        using IScrollRuleProvider = systems::scrolling::atomic::IScrollRuleProvider;
        using LayerView           = systems::view::Layer;
        using RenderContext       = systems::view::RenderContext;
        using StateProvider       = core::assembly::StateProvider;


        using SpriteManagerId   = rendering::sprite::SpriteManager::Id;

    public:
        using ScrollDir         = systems::scrolling::atomic::PageScroll::Dir;

        PlayerEntity(SpriteManagerId id);

        // Main action updates (IUpdatable)
        void Update(double /*dt*/) override;
        void TickAnimation(double dt) override;
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

        void SetViewBounds(const systems::scrolling::atomic::ViewBounds& b) noexcept;
        const WorldBounds& ViewBounds() const noexcept { return _vBounds; }

        // ===== dependency injection & configuration =====
        void SetInput(StateProvider* in) { _input = in; }
        void SetTuning(const PlayerTuning& t) { _tuning = t; }
        void SetPageOriginPx(const Vec2& p) noexcept { _pageOriginPx = p; }
        void SetTerrainProbe(ITerrainProbe* p) noexcept { _terrainProbe = p; }
        void SetLadderService(ILadderService* s) { _ladderService = s; }
        void SetScrollContext(const IScrollRuleProvider* rules, std::size_t pageIndex);
        void SetScrollRuleProvider(IScrollRuleProvider* p) noexcept { _scrollRules = p; }

        // Get scrolling request (if any) and consume it
        [[nodiscard]] std::optional<FixedScrollRequest> ConsumeScrollRequest() noexcept;

        // ===== public parameters =====
        bool onGround{ false };
        AvatarDirection facingLR{ AvatarDirection::Right };
        int  texture{ 0 };

    private:
        void refreshProbes_(PlayerContext& cx) noexcept;
        void requestScroll_(FixedScrollRequest req) noexcept;

        std::unique_ptr<IPlayerState>& FindState(AvatarStatus s)
        {
            switch (s)
            {
            case AvatarStatus::Running:   return _states[1];
            case AvatarStatus::Hovering:  return _states[2];
            case AvatarStatus::LaunchRun: return _states[3];
            case AvatarStatus::BrakeRun:  return _states[4];
            case AvatarStatus::Laddering: return _states[5];
            case AvatarStatus::Landing:   return _states[6];
            case AvatarStatus::Standing:
            default:                      return _states[0];
            }
        }

    private:
        const std::wstring kClassName{ L"PlayerEntity" };

        SpriteManagerId _id{};                                          // Sprite Id

        Vec2 _half{};                                                   // Half-size of the bounding box
        bool _collidable{ true };                                       // Whether collision is enabled
        AvatarStatus _status{ AvatarStatus::Standing };                 // Current avatar status
        std::array<std::unique_ptr<IPlayerState>, 7> _states{};

        StateProvider*             _input{};                            // Player input snapshot (This is separate from core::assembly::InputSnapshot)
        PlayerTuning               _tuning{};                           // Player tuning parameters
        AnimeStepper               _animeStepper{};                     // Animation stepper
        Probes                     _probes{ _half };                    // Collision probes
        const ITerrainProbe*       _terrainProbe{ nullptr };            // Terrain probe
        ILadderService*            _ladderService{ nullptr };           // Laddering action service
        WorldBounds                _vBounds{};                          // View boundaries
        Vec2                       _pageOriginPx{};                     // Current page origin in world px
        const IScrollRuleProvider* _scrollRules{ nullptr };             // Scroll rule provider
        std::size_t                _scrollPageIndex{ 0 };               // Current page index for scrolling
        std::optional<FixedScrollRequest> _pendingScrollReq{};          // Pending fixed scroll request
    };
}