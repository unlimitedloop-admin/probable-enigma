//==============================================================================
// 
//  Project: mm2hack
//  SoundTest.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include "test/driver/ITestDriver.h"

#include "apps/audio/AudioManager.h"

namespace mm2hack::apps::scenes
{
    class SoundTest final : public ITestDriver
    {
    public:
        SoundTest() {}
        ~SoundTest() {}

        bool Initialize() override;
        void Update() override;
        void Draw() override;
        void Finalize() override;

    private:
        bool _isPlayThisTrack = false;
    };
}