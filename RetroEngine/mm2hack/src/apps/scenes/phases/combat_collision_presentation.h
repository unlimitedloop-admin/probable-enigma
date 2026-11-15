//==============================================================================
//
//  combat_collision_presentation.h
//  Resolves entity collisions and plays the resulting combat effects.
//
//==============================================================================
#pragma once

namespace mm2hack::apps::systems::audio
{
    class AudioManager;
}

namespace mm2hack::apps::scenes::phases
{
    struct StageRuntimeContext;

    /// Resolves combat after entities move and presents the results in the same frame.
    void resolve_combat_collisions(StageRuntimeContext& ctx, systems::audio::AudioManager& audio);
}
