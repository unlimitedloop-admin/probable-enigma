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

    // Everything loaded from one enemy kind's JSON definition file.
    struct EnemyDefinition final
    {
        std::string id;
        std::string enemy_type;
        std::string name;
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
