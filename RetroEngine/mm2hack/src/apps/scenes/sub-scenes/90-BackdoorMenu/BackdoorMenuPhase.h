//==============================================================================
// 
//  Project: mm2hack
//  BackdoorMenuPhase.h
// 
//  Child scenes for BackdoorMenu scene.
// 
//==============================================================================
#pragma once

#include "BackdoorMenu.h"

#include <array>
#include <optional>
#include <span>
#include <string_view>
#include <vector>
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
            BackdoorMenuPhaseId Id() const noexcept override;

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
            BackdoorMenuPhaseId Id() const noexcept override;

        private:
            void DrawMenuItems() const;

        private:
            BackdoorMenu& owner;
            int cursorPos_{ 0 };
            algorithm::universal::MenuCursorController cursorCtl_{ {16, 16, 10}, static_cast<int>(kTopMenuTitles.size()) };
            vfx::cursor::TwinkleCursorAnimator& cursorAnim_;
        };

        static consteval std::size_t TopItemCount() noexcept { return kTopMenuTitles.size(); }

        enum class BackBehavior : unsigned char
        {
            Step = 0,   // one step back to the previous menu
            ToTop,      // back to the top menu
        };

        // Inside menu phase - displays detailed options for the selected top menu item
        class InsideMenuPhase : public IBackdoorMenuPhase
        {
        public:
            explicit InsideMenuPhase(BackdoorMenu& owner, algorithm::universal::MenuCursorController cursorCtl, int topItemIndex);

            void Update() override;
            void RenderWorld() override;
            void RenderOverlay() override;
            BackdoorMenuPhaseId Id() const noexcept override;

        private:
            using DrawHandler = void (InsideMenuPhase::*)() const;
            using ActHandler = void (InsideMenuPhase::*)() noexcept;

            struct MenuEntry
            {
                std::wstring_view label;
                bool selectable;
                ActHandler onActivate;
                int advanceLines;       // Next entry is this many lines below (for spacing)
                std::optional<int> enterSubId;
            };

            struct Page
            {
                int cursorX{ 16 };
                int firstY{ 16 };
                int lineH{ 10 };
                std::vector<MenuEntry> entries;
                std::vector<int>       selectableRows;  // Indices of selectable entries
                std::vector<int>       rowYs;           // Y positions of all entries
            };

            struct Crumb
            {
                int subId;
                int cursor;
            };

            // InsideMenu page metadata for each top menu item
            void CompleteArsenalDisplay_() const;
            void ParameterConfigurationDisplay_() const;
            void ViewerModeDisplay_() const;
            void StagesDisplay_() const;
            void RegularBootDisplay_() const;
            void SoundTestModeDisplay_() const;
            void SpriteTestDisplay_() const;
            void ResetParameterDisplay_() const;

            // NOTE: These must match the order of kTopMenuTitles
            static constexpr std::array<DrawHandler, TopItemCount()> kDrawHandlers{
                &InsideMenuPhase::CompleteArsenalDisplay_,
                &InsideMenuPhase::ParameterConfigurationDisplay_,
                &InsideMenuPhase::ViewerModeDisplay_,
                &InsideMenuPhase::StagesDisplay_,
                &InsideMenuPhase::RegularBootDisplay_,
                &InsideMenuPhase::SoundTestModeDisplay_,
                &InsideMenuPhase::SpriteTestDisplay_,
                &InsideMenuPhase::ResetParameterDisplay_
            };

            // Page construction and reflection
            void BuildPageModel_();
            void ApplyPageLayout_();

            // Input actions
            void ActivateCurrent_() noexcept; // A/START
            void GoBackToTop_() noexcept;     // B/BACK or deciding on BACK item

            static ActHandler ResolveAction_(Action a) noexcept;
            void AppendEntriesFrom_(std::span<const InsideMenuItemDesc> src);

            void NavigateInto_(int subId);
            void NavigateBack_(BackBehavior mode) noexcept;
            void BackOne_() noexcept;
            void BackToTop_() noexcept;
            void JumpToScene_() noexcept;

        private:
            BackdoorMenu& owner;
            
            int cursorPos_{ 0 };                // Selection index (index of selectableRows)
            int topItemIndex_{ 0 };             // Top menu item index
            std::vector<Crumb> insideStack_;    // Stack of inside menu crumbs for navigation

            BackBehavior defaultBack_{ BackBehavior::Step };

            algorithm::universal::MenuCursorController cursorCtl_{ {16, 16, 10}, 1 };   // Will be configured per page
            vfx::cursor::TwinkleCursorAnimator& cursorAnim_;
            Page page_;     // Current page data
        };
    }
}