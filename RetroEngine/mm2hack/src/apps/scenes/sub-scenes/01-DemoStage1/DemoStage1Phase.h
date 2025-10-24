//==============================================================================
// 
//  Project: mm2hack
//  DemoStage1Phase.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include "DemoStage1.h"

namespace mm2hack::apps::scenes
{
    namespace DemoStage1_
    {
        // Main phase - action stage scene
        class MainPhase : public IDemoStage1Phase
        {
        public:
            explicit MainPhase(DemoStage1& owner) : owner(owner) {}
            void Update() override;
            void RenderWorld() override;
            void RenderOverlay() override;
            DemoStage1PhaseId Id() const noexcept override;

        private:
            DemoStage1& owner;
        };
    }
}