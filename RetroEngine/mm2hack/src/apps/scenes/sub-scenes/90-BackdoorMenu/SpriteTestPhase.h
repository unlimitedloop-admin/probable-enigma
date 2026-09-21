//==============================================================================
//
//  Project: mm2hack
//  SpriteTestPhase.h
//
//  SPRITE TEST's pattern-select + live-preview screen for one character
//  (debug tool: eyeball a raw tile sequence before wiring it into real
//  animation/AI logic -- see BackdoorMenuCatalog.h's "SPRITE TEST" entry).
//
//==============================================================================
#pragma once

#include "BackdoorMenu.h"

#include <string>
#include <vector>

#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/ui/controls/MenuCursorController.h"
#include "apps/vfx/cursor/TwinkleCursorAnimator.h"
#include "apps/world/entity/enemy/animation/AnimationTypes.h"
#include "core/save/StateIO.h"

namespace mm2hack::apps::scenes
{
    namespace BackdoorMenu_
    {
        // Which character's sprite sheet SPRITE TEST is currently browsing.
        enum class SpriteTestCharacterId
        {
            Rockman,
            Metall,
        };

        // A single named, playable tile sequence for SPRITE TEST's animation
        // mode. Reuses animation::AnimationClip/AnimationFrame (the same
        // {tile, wait} shape the real enemy state graphs use, see
        // apps/world/entity/enemy/animation/AnimationTypes.h) rather than a
        // new type -- this is deliberately simpler than a full
        // AnimationStatePlayer: one clip, no transitions/conditions, just
        // played back linearly (see SpriteTestPhase::tickClip_()).
        struct SpriteTestPattern
        {
            std::wstring label;
            world::entity::enemy::animation::AnimationClip clip;
        };

        // SPRITE TEST's pattern-select + live-preview screen for one
        // character. Queued by InsideMenuPhase when ROCKMAN/METALL is picked
        // from the SPRITE TEST top-menu page (topItemIndex_ == 6, see
        // InsideMenuPhase::EnterSpriteTestRockman_()/EnterSpriteTestMetall_());
        // returns to that same page on BACK from the pattern list.
        //
        // Debug-only: not wired into BackdoorMenu's save-state machinery
        // (BackdoorMenuPhaseId::SpriteTest has no case in
        // BackdoorMenu.cpp's ValidatePhaseState()/Load() switches, and
        // Save() below always fails) -- a save attempted while this phase is
        // active simply fails rather than persisting a half-supported state.
        class SpriteTestPhase final : public IBackdoorMenuPhase
        {
            using MenuCursor = ui::controls::MenuCursorController;
            using CursorPointer = vfx::cursor::TwinkleCursorAnimator&;
            using SpriteId = rendering::sprite::SpriteManager::Id;

        public:
            explicit SpriteTestPhase(BackdoorMenu& owner, SpriteTestCharacterId character);

            void Update() override;
            void RenderWorld() override;
            void RenderOverlay() override;
            BackdoorMenuPhaseId Id() const noexcept override { return BackdoorMenuPhaseId::SpriteTest; }
            bool Save(core::save::StateWriter& writer) const override;
            bool Load(core::save::StateReader& reader) override;

        private:
            enum class SubState { PatternSelect, Preview };

            void goBackToCharacterSelect_() noexcept;
            void startPreview_() noexcept;
            void restartClip_() noexcept;
            void tickClip_() noexcept; // Advances the previewed clip by one frame (loop or hold on the last one)

            [[nodiscard]] const SpriteTestPattern* currentPattern_() const noexcept;
            // Tile shown before any pattern has been previewed / while the
            // list has nothing to preview -- the first frame of the first
            // pattern, or 0 if there are no patterns at all.
            [[nodiscard]] int idleTile_() const noexcept;

        private:
            BackdoorMenu& owner;
            SpriteTestCharacterId character_;
            std::wstring characterLabel_;
            SpriteId spriteId_{ static_cast<SpriteId>(-1) };

            std::vector<SpriteTestPattern> patterns_{}; // Empty if the JSON failed to load

            SubState subState_{ SubState::PatternSelect };
            MenuCursor cursorCtl_{ {16, 16, 10}, 1 };   // Item count set from patterns_.size()+1 (BACK) in the ctor
            CursorPointer cursorAnim_;

            // Live clip playback state (SubState::Preview only)
            int frameIndex_{ 0 };
            int frameElapsed_{ 0 };
        };
    }
}
