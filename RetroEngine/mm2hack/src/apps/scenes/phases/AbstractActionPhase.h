//==============================================================================
// 
//  Project: mm2hack
//  AbstractActionPhase.h
// 
//  Abstract action stage module that manages runtime context and stage script.
// 
//==============================================================================
#pragma once

#include "IPhase.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/systems/scrolling/atomic/ScrollController.h"
#include "apps/ui/productions/StageIntroUI.h"
#include "apps/world/entity/avatar/PlayerFrameOutput.h"
#include "core/save/StateIO.h"
#include "IPhaseHost.h"
#include "PhaseResult.h"
#include "StageRuntimeContext.h"

namespace mm2hack::apps::resources::parameters
{
    class Parameters;
}

namespace mm2hack::apps::world::entity::avatar
{
    class PlayerEntity;
}

namespace mm2hack::apps::world::entity
{
    struct EntityManagerState;
}

namespace mm2hack::apps::systems::audio
{
    struct SeTransportState;
}

namespace mm2hack::apps::systems::physics
{
    struct ICollider;
}

namespace mm2hack::apps::systems::combat
{
    struct IDamageable;
}

namespace mm2hack::apps::scenes::phases
{
    class IStageScript;

    // States within the action phase
    enum class ActionPhaseState : std::uint8_t
    {
        Intro,  // avatar character warp animation, and more
        Active, // main gameplay state
        Miss    // player lost (0 HP / fell into a pit): bubbles scatter, then the stage restarts.
                // Never serialized -- CanCaptureState() refuses a capture for the whole window.
    };

    // What ended the player's life -- picks how long the miss sequence runs
    // before the restart's fade-out.
    enum class MissCause : std::uint8_t
    {
        OutOfVitality,  // HP reached 0
        FellIntoPit     // dropped out of the view with no room below
    };

    enum class ActionIntroStep : std::uint8_t
    {
        Standby,
        ReadyBlink,
        WarpIn,
        Done
    };

    struct AbstractActionPhaseState final
    {
        ActionPhaseState phase{ ActionPhaseState::Intro };
        ActionIntroStep intro_step{ ActionIntroStep::Standby };
        double intro_timer{};
        bool entered{};
        bool operate{};
        foundation::math::Vec2 player_previous_position{};
        ui::productions::StageIntroUIState ready_ui{};
        systems::scrolling::atomic::ScrollControllerState scroll{};
        // Shared across every EnemyEntity in the stage; incremented once per
        // AnimationTransition::increments_shared_counter firing (see
        // EnemyEntity::SetSharedAttackCounter()/ConsumeAttackCounterIncrementRequest()).
        // Saved so a parity-gated attack pattern (e.g. Met's jump/walk split)
        // stays deterministic across a save/load, same as everything else replay cares about.
        std::uint64_t enemy_attack_pattern_counter{ 0 };

        bool Save(core::save::StateWriter& writer) const;
        bool Load(core::save::StateReader& reader);
        [[nodiscard]] bool IsValid() const noexcept;
    };

    // Abstract action phase that manages the runtime context and stage script
    class AbstractActionPhase final : public IPhase
    {
        using Vec2 = foundation::math::Vec2;

    public:
        AbstractActionPhase(std::unique_ptr<StageRuntimeContext> ctx, IStageScript* script, IPhaseHost& host) noexcept;
        ~AbstractActionPhase() override;

        AbstractActionPhase(const AbstractActionPhase&) = delete;
        AbstractActionPhase& operator=(const AbstractActionPhase&) = delete;

        // Sets up the phase with given parameters
        void Initialize(const resources::parameters::Parameters& params) override;
        // Updates the phase and returns the result
        PhaseResult Update() override;
        // Renders the world elements
        void RenderWorld() override;
        // Renders the overlay elements
        void RenderOverlay() override;
        // Enables or disables the operate phase
        void SetEnableOperatePhase(bool enable) override;
        // Gets whether the operate phase is enabled
        bool GetEnableOperatePhase() const override { return _operate; }
        [[nodiscard]] bool CanCaptureState() const noexcept;
        [[nodiscard]] bool CaptureState(AbstractActionPhaseState& state) const noexcept;
        bool CaptureEntityState(world::entity::EntityManagerState& state) const;
        bool RestoreScrollState(const AbstractActionPhaseState& state) noexcept;
        bool RestoreEntityState(
            const world::entity::EntityManagerState& state,
            const AbstractActionPhaseState& phase_state);
        bool RestoreRuntimeState(const AbstractActionPhaseState& state) noexcept;
        bool RestoreState(const AbstractActionPhaseState& state) noexcept;
        void RestoreChargePresentationState(
            const systems::audio::SeTransportState& se_state) noexcept;

    private:
        void updateIntro_();                                // Handles the intro state update
        void updateActive_();                               // Handles the active state update
        void updateMiss_();                                 // Handles the miss state update
        // Hands every enemy the position to track/attack and the shared attack
        // pattern counter for this tick (before UpdateAll()).
        void feedEnemies_(const Vec2& player_pos);
        // Entity-vs-entity collision pass plus the destruction/hit/deflect
        // presentation it triggers.
        void resolveEntityCollisions_();
        // True once the player fell fully below the current page without a
        // page scroll taking over, i.e. into a pit with no room beneath it.
        [[nodiscard]] bool hasFallenOutOfStage_(const world::entity::avatar::PlayerEntity& player) const;
        // Enters ActionPhaseState::Miss: removes the player, scatters the
        // bubbles from its position (not for a pit miss), plays the miss SE
        // and drops the BGM's pulse voices.
        void beginMiss_(world::entity::avatar::PlayerEntity& player, MissCause cause);
        // Mutes/unmutes the BGM's pulse1/pulse2 voices (the ones the miss SE
        // takes over), leaving triangle, noise and DPCM playing.
        static void setBgmMissVoicesMuted_(bool muted);
        void consumePlayerOutput_(world::entity::avatar::PlayerEntity& player); // Handles player events and spawn commands
        void updateChargePresentation_(
            const world::entity::avatar::PlayerEntity& player,
            const world::entity::avatar::ChargeStatus& charge);
        // Fires the small-explosion VFX/SFX once for every collider that was alive
        // when `colliders` was snapshotted but died during the collision pass just
        // run against it (i.e. any combat::IDamageable that just hit 0 HP).
        void spawnDestructionEffectsForTheDead_(
            const std::vector<systems::physics::ICollider*>& colliders);

        // HP of one combat::IDamageable, captured before a collision pass so a
        // non-lethal hit can be told apart from "nothing happened".
        struct DamageableHpSnapshot final
        {
            systems::combat::IDamageable* damageable{};
            int hp_before{};
        };
        [[nodiscard]] std::vector<DamageableHpSnapshot> captureDamageableHp_(
            const std::vector<systems::physics::ICollider*>& colliders) const;
        // Plays the "hit_attack" SE once if any snapshotted IDamageable took
        // damage this pass but is still alive (a lethal hit is destruction's job,
        // via spawnDestructionEffectsForTheDead_() above).
        void spawnHitEffectsForTheSurvivors_(const std::vector<DamageableHpSnapshot>& before);
        // Drains every ProjectileEntity's ConsumeDeflected() (set by
        // ProjectileEntity::OnEntityCollision() when it bounces off an
        // IDeflector, e.g. Met hidden under its helmet) and plays "defend_shot"
        // once if anything deflected this pass.
        void spawnDeflectEffectsForTheBounced_(const std::vector<systems::physics::ICollider*>& colliders);
        // Drains every alive EnemyEntity's ConsumePendingProjectileSpawns() and
        // actually Spawn()s them -- the entity itself has no EntityManager
        // access (see AnimationTransition::projectile_spawns).
        void spawnEnemyProjectiles_();
        // Drains every alive EnemyEntity's ConsumeAttackCounterIncrementRequest()
        // and bumps _enemy_attack_pattern_counter once per request -- the
        // entity itself holds no authoritative counter, only a per-frame
        // snapshot (see AnimationTransition::increments_shared_counter).
        void advanceSharedAttackCounter_();
        // Detects the scroll-lock rising/falling edge and, once per edge: clears
        // every transient effect entity and pauses SE (rising), or resumes SE
        // (falling). No-op mid-lock or mid-unlock.
        void handleScrollLockTransition_(bool locked_now);

    private:
        const std::wstring kClassName{ L"AbstractActionPhase" };

        struct IntroSequence
        {
            ActionIntroStep step{ ActionIntroStep::Standby };
            double    timer{ 0.0 };
        } _intro{};                                         // Intro sequence state

        std::unique_ptr<StageRuntimeContext> _ctx{};        // Runtime context for the stage
        std::wstring _bgm_key{};                            // Current BGM key
        IStageScript* _script{};                            // Optional stage script for custom behavior
        IPhaseHost* _host{};                                // Host for phase transitions
        bool _entered{ false };                             // Indicates if the phase has been entered
        bool _operate{ false };                             // Indicates if the operate phase is enabled (Disable at fade-in and fade-out)
        bool _charge_sound_playing{ false };                // Temporary B-hold charge sound playback state
        bool _scroll_was_locked{ false };                   // Previous frame's IsScrollLocked(), for edge detection
        world::entity::avatar::ChargePhase _charge_phase{ world::entity::avatar::ChargePhase::Idle };

        Vec2 _player_prev_pos{};                            // Previous player position, scrolling-player sync use
        std::uint64_t _enemy_attack_pattern_counter{ 0 };   // See AbstractActionPhaseState::enemy_attack_pattern_counter
        ActionPhaseState _state{ ActionPhaseState::Intro }; // Current state of the action phase
        int _miss_frames{ 0 };                              // Ticks elapsed in ActionPhaseState::Miss
        int _miss_fade_out_delay_frames{ 0 };               // Ticks this miss runs before the restart's fade-out (per MissCause)
        Vec2 _miss_origin{};                                // Where the player was lost; enemies keep targeting it during the miss
        bool _retry_requested{ false };                     // Restart already requested from the host this miss
        ui::productions::StageIntroUI _ready_ui{};          // UI for the intro sequence

        // ======== debug info ========
        int _page_index_debug{ 0 };
        double _player_pos_x_debug{ 0 };
        double _player_pos_y_debug{ 0 };
        int _player_hp_debug{ 0 };
        int _player_max_hp_debug{ 0 };
    };
}
