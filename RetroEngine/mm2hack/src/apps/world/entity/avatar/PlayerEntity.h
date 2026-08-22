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
#include "apps/systems/scrolling/atomic/ScrollTypes.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/systems/view/ViewState.h"
#include "apps/world/entity/common/AnimeStepper.h"
#include "apps/world/entity/common/FrameGate.h"
#include "apps/world/entity/common/SpawnProjectileCommand.h"
#include "apps/world/entity/common/SpawnSplashEffectCommand.h"
#include "apps/world/entity/IEntity.h"
#include "AvatarStatus.h"
#include "IPlayerState.h"
#include "PlayerContext.h"
#include "PlayerParams.h"
#include "states/AttackActionState.h"

namespace mm2hack::core::assembly
{
    class StateProvider;
}

namespace mm2hack::apps::world::entity::avatar
{
    // User player character entity
    class PlayerEntity final : public EntityBase, public systems::physics::ICollider
    {
        using AnimeStepper              = common::AnimeStepper;
        using FrameGate                 = common::FrameGate;
        using SpawnProjectileCommand    = common::SpawnProjectileCommand;
        using SpawnSplashEffectCommand  = common::SpawnSplashEffectCommand;
        using RectF                     = foundation::math::RectF;
        using Vec2                      = foundation::math::Vec2;
        using CollisionLayer            = systems::physics::CollisionLayer;
        using ILadderService            = systems::physics::ILadderService;
        using ITerrainProbe             = systems::physics::ITerrainProbe;
        using TileAttribute             = systems::physics::TileAttribute;
        using PageGridIndex             = systems::physics::PageGridIndex;
        using Probes                    = systems::physics::Probes;
        using IScrollRuleProvider       = systems::scrolling::atomic::IScrollRuleProvider;
        using LayerView                 = systems::view::Layer;
        using RenderContext             = systems::view::RenderContext;
        using StateProvider             = core::assembly::StateProvider;

        using SpriteManagerId           = rendering::sprite::SpriteManager::Id;

    public:
        using ScrollDir                 = systems::scrolling::atomic::PageScroll::Dir;

        PlayerEntity(
            SpriteManagerId id,
            SpriteManagerId weaponId,
            SpriteManagerId effectsId = static_cast<SpriteManagerId>(-1));

        // Main action updates (IUpdatable)
        void Update(const systems::view::ViewState* view, double dt) override;
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

        // Animation tick
        void TickAnimation(double dt);
        // Start intro drop state
        void BeginIntroDrop();
        // Update intro drop animation
        void UpdateIntroAnimation(double dt);
        // Update intro falling animation
        void UpdateIntroFalling(double dt);
        // Update intro landing animation
        void UpdateIntroLanding(double dt);
        // Check if intro animation is finished
        bool IsIntroFinished() const noexcept;

        // Set collidable
        void SetCollidable(bool v) noexcept;

        // Set/Get view boundaries
        void SetViewBounds(const systems::scrolling::atomic::ViewBounds& b) noexcept;
        const WorldBounds& ViewBounds() const noexcept { return _v_bounds; }

        // ===== dependency injection & configuration =====
        void SetInput(StateProvider* in) { _input = in; }
        void SetTuning(const PlayerTuning& t);
        void SetPageOriginPx(const Vec2& p) noexcept { _page_origin_px = p; }
        void SetTerrainProbe(ITerrainProbe* p) noexcept { _terrain_probe = p; }
        void SetLadderService(ILadderService* s) { _ladder_service = s; }
        void SetScrollContext(const IScrollRuleProvider* rules, std::size_t pageIndex);
        void SetScrollRuleProvider(IScrollRuleProvider* p) noexcept { _scroll_rules = p; }
        void SetEntityContext(const ExPlayerContextForEntity& cx) noexcept { _entityContext = cx; }

        // Get scrolling request (if any) and consume it
        [[nodiscard]] std::optional<FixedScrollRequest> ConsumeScrollRequest() noexcept;

        // Parameter that acts as a dedicated flag for fixed page scrolling.
        void SetFixedPageScrollAvailable(bool v) noexcept { _fixed_scroll_available = v; }
        // Spawn projectile command handling
        std::optional<SpawnProjectileCommand> TakeSpawnProjectile();
        // Spawn splash effect command handling
        std::optional<SpawnSplashEffectCommand> TakeSpawnSplashEffect();

        // ===== public parameters =====
        bool onGround{ false };                                     // Is on the ground?
        AvatarDirection facingLR{ AvatarDirection::Right };         // Player direction: -1: left, +1: right
        int texture{ 0 };                                           // baseTexture + attackTexture
        int baseTexture{ 0 };                                       // Base texture index (without animation offset)
        int attackTexture{ 0 };                                     // Current attack texture (for rock buster drawing)

    private:
        void composeFinalTexture_() noexcept;                       // Set current avatar tile number
        void refreshProbes_(PlayerContext& cx) noexcept;            // Refresh collision all probes
        void updateEnvironment_() noexcept;                         // Update current physical environment
        void selectTuning_() noexcept;                              // Select tuning for current environment
        [[nodiscard]] bool shouldSkipUnderwaterPhysics_() noexcept; // Check if underwater physics should be skipped (for performance)

        void requestScroll_(FixedScrollRequest req) noexcept;       // Request fixed page scrolling

        std::unique_ptr<IPlayerState>& findState_(AvatarStatus s)
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

        static constexpr std::array<IntroFrame, 9> kLandingFrames
        {
            IntroFrame{ STile::IntroDropA, 5.0 / 60.0 },
            IntroFrame{ STile::IntroDropB, 5.0 / 60.0 },
            IntroFrame{ STile::IntroDropC, 5.0 / 60.0 },
            IntroFrame{ STile::IntroDropD, 5.0 / 60.0 },
            IntroFrame{ STile::IntroDropE, 5.0 / 60.0 },
            IntroFrame{ STile::IntroDropF, 16.0 / 60.0 },
            IntroFrame{ STile::IntroDropG, 4.0 / 60.0 },
            IntroFrame{ STile::IntroDropH, 3.0 / 60.0 },
            IntroFrame{ STile::IntroDropI, 13.0 / 60.0 }
        };                                                          // Landing animation frames in the intro drop sequence

        SpriteManagerId _id{};                                      // Sprite Id
        SpriteManagerId _effects_id{};                              // Effect sprite Id
        Vec2 _half{};                                               // Half-size of the bounding box
        bool _collidable{ true };                                   // Whether collision is enabled
        AvatarStatus _status{ AvatarStatus::Standing };             // Current avatar status
        std::array<std::unique_ptr<IPlayerState>, 7> _states{};     // Basic behavior states array
        std::unique_ptr<states::AttackActionState> _attackAction{}; // Attack action state handler
        states::RockBusterDrawInfo _rock_buster{};                  // Rock Buster drawing info

        StateProvider* _input{};                                    // Player input snapshot (This is separate from core::assembly::InputSnapshot)
        PlayerTuning _normal_tuning{};                              // Player tuning parameters
        PlayerTuning _underwater_tuning{};                          // Underwater tuning parameters
        const PlayerTuning* _current_tuning{ nullptr };             // Pointer to current tuning (switches between normal and underwater)
        PlayerEnvironment _environment{ PlayerEnvironment::Normal };// Current physical environment
        IntroDropState _intro_states{};                             // Appearing the player on stage parameters
        states::AttackTuning _attack_tuning{};                      // Attack action tuning parameters
        AnimeStepper _anime_stepper{};                              // Animation stepper
        Probes _probes{ _half };                                    // Collision probes
        const ITerrainProbe* _terrain_probe{ nullptr };             // Terrain probe
        ILadderService* _ladder_service{ nullptr };                 // Laddering action service
        FrameGate _underwater_physics_gate{};                       // Underwater physics gate
        bool _jump_buffered{ false };                               // Jump edge latched across an underwater skip-physics tick (see PlayerContext::jumpEdge)

        WorldBounds _v_bounds{};                                    // View boundaries

        Vec2 _page_origin_px{};                                     // Current page origin in world px
        const IScrollRuleProvider* _scroll_rules{ nullptr };        // Scroll rule provider
        std::size_t _scroll_page_index{ 0 };                        // Current page index for scrolling
        std::optional<FixedScrollRequest> _pending_scroll_req{};    // Pending fixed scroll request
        bool _fixed_scroll_available{ false };                      // Is fixed-page scrolling available?

        ExPlayerContextForEntity _entityContext{};                  // Extended context for entity-level data
        std::optional<SpawnProjectileCommand> _spawnProjectile{};   // Pending projectile spawn command
        std::optional<SpawnSplashEffectCommand> _spawn_splash_effect{}; // Pending splash effect spawn command
    };
}
