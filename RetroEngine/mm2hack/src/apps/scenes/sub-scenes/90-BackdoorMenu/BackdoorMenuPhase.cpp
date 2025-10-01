#include "pch.h"

#include "BackdoorMenuPhase.h"

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
            auto& resource = owner.Resource();
            auto& input = owner.Input();
            auto startPressed = input->JustPressed(JPBTN::START) || input->JustPressed(JPBTN::A);

            resource->UpdateEffects();   // Update fade effects

            if (startPressed)
            {
                owner.SetPhase(std::make_unique<TopMenuPhase>(owner));
            }
        }

        void CreditPhase::RenderWorld()
        {
            auto& fonts = owner.Resource()->GetFontTileManager();
            fonts.DrawTextImage(L"BACKDOOR MENU", 76, 56);
            fonts.DrawTextImage(L" PRESS START ", 76, 96);
        }

        void CreditPhase::RenderOverlay()
        {
            // Nothing to render in overlay for this phase.
        }

        bool CreditPhase::IsComplete() const
        {
            return false; // Always returns false; phase completion is handled externally.
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
        }

        void TopMenuPhase::RenderOverlay()
        {
            // Nothing to render in overlay for this phase.
        }

        bool TopMenuPhase::IsComplete() const
        {
            return false; // Always returns false; phase completion is handled externally.
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

        bool InsideMenuPhase::IsComplete() const
        {
            return false; // Always returns false; phase completion is handled externally.
        }
    }
}