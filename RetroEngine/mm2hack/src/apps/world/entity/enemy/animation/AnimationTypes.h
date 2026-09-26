//==============================================================================
//
//  Project: mm2hack
//  AnimationTypes.h
//
//  Pure data model for an enemy's animation state graph and palette presets,
//  as loaded from its JSON definition file (see EnemyDefinitionLoader). Kept
//  free of any engine/rendering dependency so it can be unit-tested and
//  reused by tools (e.g. a future stage editor) without pulling in DxLib.
//
//==============================================================================
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "apps/foundation/math/CoordinateTypes.h"

namespace mm2hack::apps::world::entity::enemy::animation
{
    // Recognized transition triggers, all driven by whatever
    // AnimationConditionInputs the caller passes to Tick() each frame (see
    // EnemyEntity, which feeds Grounded/Airborne from combat::
    // SimpleGravityBody and PlayerNear from whoever last called
    // EnemyEntity::SetPlayerPosition()).
    enum class AnimationCondition : std::uint8_t
    {
        Timer,          // Fires after `param_frames` ticks spent in this state
        ClipFinished,   // Fires the instant a non-looping clip completes its last frame
        PlayerNear,     // Fires while |player - entity| <= (param_x, param_y), both axes
        Grounded,       // Fires on a tick where AnimationConditionInputs::grounded is true
        Airborne,       // Fires on a tick where AnimationConditionInputs::grounded is false
    };

    // Optional extra gate ANDed onto a transition's own `condition` (see
    // AnimationTransition::required_parity) -- lets two transitions with the
    // same trigger stay mutually exclusive based on AnimationConditionInputs::
    // shared_counter instead of picking randomly. Deliberately not a
    // condition of its own: it never fires by itself (a state with only a
    // parity-gated transition and no matching base condition would never
    // leave), it only narrows one that already would.
    enum class CounterParity : std::uint8_t
    {
        Any,    // No gating -- the transition's own condition is the only requirement
        Even,
        Odd
    };

    // External signals AnimationStatePlayer::Tick() needs to evaluate
    // conditions it can't determine from the clip data alone.
    struct AnimationConditionInputs final
    {
        bool grounded{ true };
        // Absolute horizontal/vertical distance to the player, in px. Left at
        // this huge sentinel when the caller has no player position to report
        // yet, so a PlayerNear transition simply never fires rather than
        // firing spuriously against a stale (0,0).
        double player_dx{ 1'000'000.0 };
        double player_dy{ 1'000'000.0 };
        // Current value of a counter shared across every enemy in the stage,
        // incremented once per AnimationTransition::increments_shared_counter
        // firing (see EnemyEntity::SetSharedAttackCounter() /
        // AbstractActionPhase, which owns the real, save-stated value and
        // feeds it in each frame the same way it feeds player_dx/dy). Used to
        // alternate between two otherwise-identical triggers (see
        // CounterParity) deterministically -- replay-safe, unlike an RNG roll.
        std::uint64_t shared_counter{ 0 };
    };

    // One frame of a clip: which tile to show, and how long (in ticks/frames,
    // matching every other frame-stepped effect in this codebase -- see
    // ChargeEffectEntity::kFrameDurations) to hold it before advancing.
    struct AnimationFrame final
    {
        // Sentinel for "never auto-advance"; only a transition can move the
        // state machine off a frame using this (e.g. a jump's airborne pose,
        // held until a future `Grounded` transition fires).
        static constexpr int kHoldFrames{ -1 };

        int tile{ 0 };
        int wait_frames{ 1 };
    };

    struct AnimationClip final
    {
        std::vector<AnimationFrame> frames;
        bool loop{ false };
    };

    // One shot fired the instant a transition carrying it fires. `angle_deg`:
    // 0 = straight along the entity's current facing, positive = rotates
    // toward +Y (down, since screen Y grows downward), negative = up. The
    // horizontal component mirrors with facing; the vertical component
    // doesn't (an "up-45" shot is always up, whichever way the entity faces).
    struct ProjectileSpawnSpec final
    {
        double angle_deg{ 0.0 };
        double speed_px_per_frame{ 0.0 };
    };

    struct AnimationTransition final
    {
        AnimationCondition condition{ AnimationCondition::Timer };
        int param_frames{ 0 };     // Timer: frames to wait in-state before firing
        double param_x{ 0.0 };     // PlayerNear: horizontal trigger half-range, px
        double param_y{ 0.0 };     // PlayerNear: vertical trigger half-range, px
        std::string to_state;      // Target AnimationState::id
        // 0 = no jump (the common case -- most transitions are just a pose
        // change). Non-zero: the instant this transition fires, the entity's
        // SimpleGravityBody gets Jump()'d with this value (negative = upward,
        // same convention as PlayerTuning::jumpImpulse). Lets two transitions
        // into the *same* airborne state stay distinct -- e.g. Met's `walk`
        // has one `airborne`-triggered transition to `falling` with no
        // impulse (stepped off a ledge) and one `timer`-triggered transition
        // to the same `falling` state with an impulse (a deliberate hop).
        double jump_impulse{ 0.0 };
        // Empty (the common case) = fires nothing. Non-empty: the instant this
        // transition fires, the entity spawns one projectile per entry here.
        std::vector<ProjectileSpawnSpec> projectile_spawns{};
        // Any (the common case): this transition fires whenever `condition`
        // does. Even/Odd: also requires AnimationConditionInputs::
        // shared_counter to have that parity -- e.g. two PlayerNear
        // transitions out of the same state, one Even one Odd, deterministically
        // alternate between two attack patterns across repeated triggers
        // instead of always picking the same one (see CounterParity).
        CounterParity required_parity{ CounterParity::Any };
        // False (the common case): no side effect. True: the instant this
        // transition fires, the shared counter above increments by one (via
        // EnemyEntity's pending-increment poll, same fire-and-drain shape as
        // jump_impulse/projectile_spawns) -- so the *next* parity-gated
        // decision flips. Combined with required_parity, this is what turns a
        // fixed Any/Even/Odd split into a clean A/B/A/B alternation.
        bool increments_shared_counter{ false };
    };

    struct AnimationState final
    {
        std::string id;
        AnimationClip clip;
        std::vector<AnimationTransition> transitions;
        // Whether the entity's own locomotion (e.g. EnemyEntity's left-right
        // patrol) should run while this state is active. A "hidden"/peeking
        // state, for instance, sets this false so the creature holds still.
        bool allow_movement{ true };
        // Multiplies the entity's own base horizontal speed while this state
        // is active (e.g. faster horizontal drift mid-jump than while walking).
        double move_speed_multiplier{ 1.0 };
        // Whether the entity re-aims its facing at the player's current
        // position while this state is active (see EnemyEntity::Update()).
        // True by default (e.g. an idle/waiting state should always be ready
        // to aim); a committed attack -- Met's rise/jump/cooldown -- sets this
        // false so the facing it locked in when the attack started (and every
        // shot/hop direction derived from it) can't flip mid-attack just
        // because the player crossed to the other side.
        bool track_player_facing{ true };
        // False (the common case): a hit is a normal hit. True: while this
        // state is active, the entity takes no damage from an incoming
        // attack at all -- the attacking collider deflects instead (see
        // systems::physics::IDeflector, EnemyEntity::DeflectsAttacks()).
        // Met's `idle` (still hidden under its helmet) sets this true.
        bool deflects_attacks{ false };
    };

    // One creature's full animation graph, loaded once from its JSON pattern
    // file and kept alive for the process's lifetime (see EnemyDefinitionCatalog).
    struct EnemyAnimationDef final
    {
        std::string initial_state;
        std::vector<AnimationState> states;

        // Linear scan (state lists are small -- a handful of entries); returns
        // -1 if no state with this id exists.
        [[nodiscard]] int FindStateIndex(const std::string& state_id) const noexcept
        {
            for (std::size_t i = 0; i < states.size(); ++i)
            {
                if (states[i].id == state_id)
                {
                    return static_cast<int>(i);
                }
            }
            return -1;
        }
    };

    struct EnemyPaletteMapping final
    {
        int source_index{ 0 };  // NES palette index (0-63) currently present in the art
        int target_index{ 0 };  // NES palette index (0-63) to replace it with
    };

    // A named recolor. Empty `mappings` means "the sheet's original colors" --
    // by convention, index 0 in EnemyDefinition::palette_presets should be such
    // an entry (see EnemyDefinitionLoader / DemoStage2's loading code).
    struct EnemyPalettePreset final
    {
        std::string id;
        std::vector<EnemyPaletteMapping> mappings;
    };

    // A kind's own fixed capabilities -- the "abilities" block of its JSON
    // definition. Deliberately kind-wide only: WHERE an enemy is placed (and
    // its facing/color there) belongs to the stage's placement data, never
    // here, and the same kind never varies these per placement. A future
    // difficulty setting is meant to scale these at spawn time rather than
    // fork them per stage. Defaults below apply to any key the JSON omits.
    struct EnemyAbilities final
    {
        // 0 = invincible (every weapon immune, HP never moves); N = dies after
        // N hits' worth of accumulated normal-shot power.
        int hp{ 0x01 };
        int contact_power{ 0x01 };      // Damage dealt by touching its body (never while deflecting)
        int projectile_power{ 0x01 };   // Damage carried by each shot its animation graph fires

        // Base horizontal speed for whatever locomotion the current animation
        // state allows (AnimationState::move_speed_multiplier scales it).
        double move_speed_px_per_sec{ 20.0 };
        // Multiplies both the shared gravity and terminal velocity (see
        // EnemyEntity::kDefaultGravityPerFrame), so heavier/floatier kinds
        // fall faster/slower without a second knob.
        double gravity_scale{ 1.0 };

        foundation::math::Vec2 sprite_half_size{ 8.0, 8.0 };    // Render anchor / probe footprint
        foundation::math::Vec2 hitbox_half_size{ 8.0, 8.0 };    // Entity-collision box, independent of the sprite
        double hitbox_offset_y{ 0.0 };                          // Shifts the hit box's center down (+) from pos
        // Added to the current tile while facing left, for sheets with
        // separate mirrored tiles (0 if symmetric / no mirrored set).
        int facing_texture_offset_left{ 0x00 };

        foundation::math::Vec2 projectile_hit_half_size{ 2.0, 2.0 }; // Each shot's hit box
        double projectile_spawn_offset_y{ 0.0 };                     // Muzzle offset from pos (+ = down)
    };

    // Everything loaded from one enemy kind's JSON definition file.
    struct EnemyDefinition final
    {
        std::string id;
        std::string enemy_type;
        std::string name;
        EnemyAbilities abilities;
        EnemyAnimationDef animation;
        std::vector<EnemyPalettePreset> palette_presets;

        [[nodiscard]] int FindPaletteIndex(const std::string& preset_id) const noexcept
        {
            for (std::size_t i = 0; i < palette_presets.size(); ++i)
            {
                if (palette_presets[i].id == preset_id)
                {
                    return static_cast<int>(i);
                }
            }
            return -1;
        }
    };
}
