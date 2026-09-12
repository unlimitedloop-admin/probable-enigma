//==============================================================================
// 
//  Project: mm2hack
//  OuterSpaceBG.h
// 
//  Sample animation of outer space background.
// 
//==============================================================================
#pragma once

#include "test/driver/ITestDriver.h"

#include <string>
#include "apps/vfx/stareffects/BgStarField.h"

namespace mm2hack::apps::scenes
{
    class OuterSpaceBG final : public ITestDriver
    {
    public:
        OuterSpaceBG() {}
        ~OuterSpaceBG() {}
        bool Initialize() override;
        void Update() override;
        void RenderWorld() override;
        void RenderOverlay() override;
        void Finalize() override;

    private:
        const std::wstring kClassName{ L"OuterSpaceBG" };

        vfx::stareffects::BgStarField _starField;
    };
}