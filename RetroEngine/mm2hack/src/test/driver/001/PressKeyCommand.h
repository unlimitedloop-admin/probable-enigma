//==============================================================================
// 
//  Project: mm2hack
//  PressKeyCommand.h
// 
//  This is a keyboard input test.
// 
//==============================================================================
#pragma once

#include "test/driver/ITestDriver.h"

namespace mm2hack::apps::scenes
{
    class PressKeyCommand final : public ITestDriver
    {
    public:
        PressKeyCommand() {}
        ~PressKeyCommand() {}

        bool Initialize() override;
        void Update() override;
        void RenderWorld() override {}
        void RenderOverlay() override {}
        void Finalize() override;
    };
}