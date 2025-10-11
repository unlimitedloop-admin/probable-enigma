#include "pch.h"

#include "BackdoorMenuPhase.h"

#include "apps/scenes/PhaseFadeController.h"
#include "BackdoorMenu.h"
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
            auto startPressed = input->JustPressed(JPBTN::START) || input->JustPressed(JPBTN::A);

            if (startPressed)
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
        }

        void TopMenuPhase::RenderWorld()
        {
            owner.StarField().DrawStars();

            // TODO: Implement top menu list rendering here.
        }

        void TopMenuPhase::RenderOverlay()
        {
            // Nothing to render in overlay for this phase.
        }

        PhaseId TopMenuPhase::Id() const noexcept
        {
            return PhaseId::TopMenu;
        }

        //==============================================================================
        //
        //  InsideMenuPhase
        //
        //==============================================================================
        void InsideMenuPhase::Update()
        {
        }

        void InsideMenuPhase::RenderWorld()
        {
        }

        void InsideMenuPhase::RenderOverlay()
        {
            // Nothing to render in overlay for this phase.
        }

        PhaseId InsideMenuPhase::Id() const noexcept
        {
            return PhaseId::InsideMenu;
        }
    }
}