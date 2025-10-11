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
            explicit TopMenuPhase(BackdoorMenu& owner) : owner(owner) {}
            void Update() override;
            void RenderWorld() override;
            void RenderOverlay() override;
            PhaseId Id() const noexcept override;

        private:
            BackdoorMenu& owner;
            int cursorPos{ 0 };
        };

        // Inside menu phase - displays detailed settings for the item selected in the top menu
        class InsideMenuPhase : public IBackdoorMenuPhase
        {
        public:
            explicit InsideMenuPhase(BackdoorMenu& owner) : owner(owner) {}
            void Update() override;
            void RenderWorld() override;
            void RenderOverlay() override;
            PhaseId Id() const noexcept override;

        private:
            BackdoorMenu& owner;
            int cursorPos{ 0 };
            int topItemIndex{ 0 };
        };
    }
}