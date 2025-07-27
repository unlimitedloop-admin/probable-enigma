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

#include "apps/audio/AudioConfigLoader.h"
#include "apps/audio/BgmManager.h"
#include "apps/audio/ChannelManager.h"
#include "apps/audio/SeManager.h"

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
        audio::ChannelManager _bgmChannels{ 5 };            // Default 5 channels for BGM
        audio::BgmManager _bgmManager{ _bgmChannels };      // BGM manager for handling background music
        audio::SeManager _seManager{ _bgmChannels, 8 };     // Default 8 channels for SE
        audio::AudioConfigLoader _configLoader;             // Load BGM and SE configurations
    };
}