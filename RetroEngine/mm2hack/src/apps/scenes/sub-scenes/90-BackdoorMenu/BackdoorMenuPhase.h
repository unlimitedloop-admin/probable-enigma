//==============================================================================
// 
//  Project: mm2hack
//  BackdoorMenuPhase.h
// 
//  Child scenes for BackdoorMenu scene.
// 
//==============================================================================
#pragma once

#include "BackdoorMenu.h"   // Including IBackdoorMenuPhase

#include <array>
#include "apps/algorithm/universal/MenuCursorController.h"
#include "apps/vfx/cursor/TwinkleCursorAnimator.h"
#include "BackdoorMenuCatalog.h"

namespace mm2hack::apps::scenes
{
    namespace BackdoorMenu_
    {
        // Credit phase - initial phase showing credits and waiting for user input
        class CreditPhase : public IBackdoorMenuPhase
        {
        public:
            explicit CreditPhase(BackdoorMenu& owner) : owner(owner) {}
            void Update() override;
            void RenderWorld() override;
            void RenderOverlay() override;
            PhaseId Id() const noexcept override;

        private:
            BackdoorMenu& owner;
        };

        // Top menu phase - displays a menu for selecting each scene in the mm2hack
        class TopMenuPhase : public IBackdoorMenuPhase
        {
        public:
            explicit TopMenuPhase(BackdoorMenu& owner) : owner(owner), cursorAnim_(owner.Cursor()) {}
            explicit TopMenuPhase(BackdoorMenu& owner, int cursorPos) : owner(owner), cursorPos_(cursorPos), cursorAnim_(owner.Cursor())
            {
                cursorCtl_.SetIndex(cursorPos_);    // case when coming from InsideMenuPhase
            }

            void Update() override;
            void RenderWorld() override;
            void RenderOverlay() override;
            PhaseId Id() const noexcept override;

        private:
            void DrawMenuItems() const;

        private:
            BackdoorMenu& owner;
            int cursorPos_{ 0 };
            algorithm::universal::MenuCursorController cursorCtl_{ {16, 16, 10}, kTopMenuTitles.size() };
            vfx::cursor::TwinkleCursorAnimator& cursorAnim_;
        };


        static consteval std::size_t TopItemCount() noexcept { return kTopMenuTitles.size(); }

        // Inside menu phase - displays detailed settings for the item selected in the top menu
        class InsideMenuPhase : public IBackdoorMenuPhase
        {
        public:
            explicit InsideMenuPhase(BackdoorMenu& owner, algorithm::universal::MenuCursorController cursorCtl, int topItemIndex);

            void Update() override;
            void RenderWorld() override;
            void RenderOverlay() override;
            PhaseId Id() const noexcept override;

        private:
            void CompleteArsenalDisplay_() const;
            void ParameterConfigurationDisplay_() const;
            void ViewerModeDisplay_() const;
            void StagesDisplay_() const;
            void RegularBootDisplay_() const;
            void SoundTestModeDisplay_() const;
            void SpriteTestDisplay_() const;
            void ResetParameterDisplay_() const;

            using Handler = void (InsideMenuPhase::*)() const;

            // NOTE: Ensure the order matches kTopMenuTitles in BackdoorMenuCatalog.h!
            static constexpr std::array<Handler, TopItemCount()> kHandlers{
                &InsideMenuPhase::CompleteArsenalDisplay_,
                &InsideMenuPhase::ParameterConfigurationDisplay_,
                &InsideMenuPhase::ViewerModeDisplay_,
                &InsideMenuPhase::StagesDisplay_,
                &InsideMenuPhase::RegularBootDisplay_,
                &InsideMenuPhase::SoundTestModeDisplay_,
                &InsideMenuPhase::SpriteTestDisplay_,
                &InsideMenuPhase::ResetParameterDisplay_
            };

            struct PageMeta { int cursorX; int firstY; int lineH; int selectableCount; };

            static constexpr PageMeta kPageMetas[] = {
                {16, 36, 10, 1},  // Complete Arsenal
                {30, 16, 10, 0},  // Parameter Configuration
                {30, 16, 10, 0},  // Viewer Mode
                {30, 16, 10, 0},  // Stages
                {30, 16, 10, 0},  // Regular Boot
                {30, 16, 10, 0},  // Sound Test Mode
                {30, 16, 10, 0},  // Sprite Test
                {30, 16, 10, 0}   // Reset Parameter
            };

            void DispatchSelected_() const noexcept;

        private:
            BackdoorMenu& owner;
            int cursorPos_{ 0 };
            int topItemIndex_{ 0 };
            algorithm::universal::MenuCursorController cursorCtl_{ {16, 16, 10}, 1 };   // Dummy initialization
            vfx::cursor::TwinkleCursorAnimator& cursorAnim_;
            int frameCounter_{ -1 };    // Unused.
        };
    }
}