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
    // Recognized transition triggers. PlayerNear is still reserved (fires
    // never) until an entity has access to the player's position -- see
    // AnimationStatePlayer::Tick(). Grounded/Airborne are implemented, driven
    // by whatever AnimationConditionInputs the caller passes to Tick() each
    // frame (see EnemyEntity, which feeds it from combat::SimpleGravityBody).
    enum class AnimationCondition : std::uint8_t
    {
        Timer,          // Fires after `param_frames` ticks spent in this state
        ClipFinished,   // Fires the instant a non-looping clip completes its last frame
        PlayerNear,     // Reserved: player within (param_x, param_y) of the entity
        Grounded,       // Fires on a tick where AnimationConditionInputs::grounded is true
        Airborne,       // Fires on a tick where AnimationConditionInputs::grounded is false
    };

    // External signals AnimationStatePlayer::Tick() needs to evaluate
    // conditions it can't determine from the clip data alone. Extend this as
    // more conditions graduate from "reserved" to implemented.
    struct AnimationConditionInputs final
    {
        bool grounded{ true };
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

    struct AnimationTransition final
    {
        AnimationCondition condition{ AnimationCondition::Timer };
        int param_frames{ 0 };     // Timer: frames to wait in-state before firing
        double param_x{ 0.0 };     // PlayerNear (reserved): horizontal trigger half-range, px
        double param_y{ 0.0 };     // PlayerNear (reserved): vertical trigger half-range, px
        std::string to_state;      // Target AnimationState::id
    };

    struct AnimationState final
    {
        std::string id;
        AnimationClip clip;
        std::vector<AnimationTransition> transitions;
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
