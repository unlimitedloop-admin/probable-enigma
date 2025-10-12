#include "pch.h"

#include "BackdoorMenuPhase.h"

#include <array>
#include "apps/algorithm/universal/MenuCursorController.h"
#include "apps/scenes/PhaseFadeController.h"
#include "BackdoorMenu.h"
#include "BackdoorMenuCatalog.h"
#include "input/Jpbtn.h"

namespace mm2hack::apps::scenes
{
    namespace BackdoorMenu_
    {
        //==============================================================================
        //
        //  CreditPhase
        //
        //==============================================================================
        void CreditPhase::Update()
        {
            auto& input = owner.Input();
            auto verified = input->JustPressed(JPBTN::START) || input->JustPressed(JPBTN::A);

            if (verified)
            {
                auto& audio = owner.Resource()->GetAudioManager();
                audio.PlaySe(L"enter_ring");

                PhaseFadePlan next(
                    5,   // preBlackHold
                    20,  // fadeInFrames
                    0,   // preFadeOutHold
                    20,  // fadeOutFrames
                    0,   // postBlackHold
                    FadeLayerMask::All // layers
                );
                owner.QueuePhase(std::make_unique<TopMenuPhase>(owner), next);
            }
        }

        void CreditPhase::RenderWorld()
        {
            auto& fonts = owner.Resource()->GetFontTileManager();
            fonts.DrawTextImage(L"   MM2HACK DEMO   ", 60, 46);
            fonts.DrawTextImage(L"2024-2026 SIRIUS X", 60, 66);
            fonts.DrawTextImage(L"  BACKDOOR MENU   ", 60, 106);
            fonts.DrawTextImage(L" PRESS START KEY  ", 60, 136);
        }

        void CreditPhase::RenderOverlay()
        {
            // Nothing to render in overlay for this phase.
        }

        PhaseId CreditPhase::Id() const noexcept
        {
            return PhaseId::Credit;
        }

        //==============================================================================
        //
        //  TopMenuPhase
        //
        //==============================================================================
        void TopMenuPhase::Update()
        {
            owner.StarField().UpdateStars();
            owner.Cursor().Update();

            auto& input = owner.Input();
            auto verified = input->JustPressed(JPBTN::START) || input->JustPressed(JPBTN::A);
            if (input->JustPressed(JPBTN::DOWN)) { cursorCtl_.Move(+1); }
            if (input->JustPressed(JPBTN::UP)) { cursorCtl_.Move(-1); }
            cursorPos_ = cursorCtl_.Index();

            if (verified)
            {
                auto& audio = owner.Resource()->GetAudioManager();
                audio.PlaySe(L"plink_ring");

                PhaseFadePlan next(
                    5,   // preBlackHold
                    20,  // fadeInFrames
                    0,   // preFadeOutHold
                    20,  // fadeOutFrames
                    0,   // postBlackHold
                    FadeLayerMask::All // layers
                );
                owner.QueuePhase(std::make_unique<InsideMenuPhase>(owner, cursorCtl_, cursorPos_), next);
            }
        }

        void TopMenuPhase::RenderWorld()
        {
            owner.StarField().DrawStars();
            owner.Cursor().DrawAt(cursorCtl_.CursorX(), cursorCtl_.CursorY());
            DrawMenuItems();
        }

        void TopMenuPhase::RenderOverlay()
        {
            // Nothing to render in overlay for this phase.
        }

        PhaseId TopMenuPhase::Id() const noexcept
        {
            return PhaseId::TopMenu;
        }

        void TopMenuPhase::DrawMenuItems() const
        {
            auto& fonts = owner.Resource()->GetFontTileManager();
            int y = 16;
            for (const auto& item : kTopMenuTitles)
            {
                auto labels = std::wstring(item);
                fonts.DrawTextImage(labels, 30, y);
                y += 10;
            }
        }

        //==============================================================================
        //
        //  InsideMenuPhase
        //
        //==============================================================================
        InsideMenuPhase::InsideMenuPhase(BackdoorMenu& owner, algorithm::universal::MenuCursorController cursorCtl, int topItemIndex)
            : owner(owner), cursorCtl_(cursorCtl), cursorAnim_(owner.Cursor()), topItemIndex_(topItemIndex)
        {
            const auto meta = kPageMetas[static_cast<std::size_t>(topItemIndex_)];
            cursorCtl_.SetLayout({ meta.cursorX, meta.firstY, meta.lineH });
            cursorCtl_.SetItemCount(meta.selectableCount);
            cursorCtl_.SetIndex(std::min(cursorCtl_.Index(), meta.selectableCount - 1));
        }
        
        void InsideMenuPhase::Update()
        {
            owner.StarField().UpdateStars();
            owner.Cursor().Update();

            auto& input = owner.Input();
            auto verified = input->JustPressed(JPBTN::START) || input->JustPressed(JPBTN::A);
            auto cancelled = input->JustPressed(JPBTN::B) || input->JustPressed(JPBTN::BACK);

            if (verified)
            {
                auto& audio = owner.Resource()->GetAudioManager();
                audio.PlaySe(L"plink_ring");
                // TODO: Handle verification action based on the selected item.
            }

            if (cancelled)
            {
                auto& audio = owner.Resource()->GetAudioManager();
                audio.PlaySe(L"plink_ring");

                PhaseFadePlan next(
                    5,   // preBlackHold
                    20,  // fadeInFrames
                    0,   // preFadeOutHold
                    20,  // fadeOutFrames
                    0,   // postBlackHold
                    FadeLayerMask::All // layers
                );
                owner.QueuePhase(std::make_unique<TopMenuPhase>(owner, topItemIndex_), next);
            }
        }

        void InsideMenuPhase::RenderWorld()
        {
            owner.StarField().DrawStars();
            owner.Cursor().DrawAt(cursorCtl_.CursorX(), cursorCtl_.CursorY());

            // Call the appropriate display function based on the selected item.
            DispatchSelected_();
        }

        void InsideMenuPhase::RenderOverlay()
        {
            // Nothing to render in overlay for this phase.
        }

        PhaseId InsideMenuPhase::Id() const noexcept
        {
            return PhaseId::InsideMenu;
        }

        void InsideMenuPhase::DispatchSelected_() const noexcept
        {
            if (topItemIndex_ < 0 || topItemIndex_ >= static_cast<int>(kHandlers.size())) { return; }

            const Handler fn = kHandlers[static_cast<std::size_t>(topItemIndex_)];
            (this->*fn)();  // Callback to the appropriate display func.
        }

        void InsideMenuPhase::CompleteArsenalDisplay_() const
        {
            auto& fonts = owner.Resource()->GetFontTileManager();
            fonts.DrawTextImage(L"ALL WEAPONS UNLOCKED.", 30, 16);
            fonts.DrawTextImage(L"BACK", 30, 36);
        }

        // TODO: Implement these display functions
        void InsideMenuPhase::ParameterConfigurationDisplay_() const
        {
        }
        void InsideMenuPhase::ViewerModeDisplay_() const
        {
        }
        void InsideMenuPhase::StagesDisplay_() const
        {
        }
        void InsideMenuPhase::RegularBootDisplay_() const
        {
        }
        void InsideMenuPhase::SoundTestModeDisplay_() const
        {
        }
        void InsideMenuPhase::SpriteTestDisplay_() const
        {
        }
        void InsideMenuPhase::ResetParameterDisplay_() const
        {
        }
    }
}