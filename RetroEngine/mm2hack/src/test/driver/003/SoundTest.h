//==============================================================================
// 
//  Project: mm2hack
//  SoundTest.h
// 
//  Sound test scene for testing sound functionalities.
// 
//==============================================================================
#pragma once

#include "test/driver/ITestDriver.h"

namespace mm2hack::apps::scenes
{
    class SoundTest final : public ITestDriver
    {
    public:
        SoundTest() {}
        ~SoundTest() {}

        bool Initialize() override;
        void Update() override;
        void RenderWorld() override;
        void RenderOverlay() override;
        void Finalize() override;

    private:
        bool _isPlayThisTrack = false;
    };
}