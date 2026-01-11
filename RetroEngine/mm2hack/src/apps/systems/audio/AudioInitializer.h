//==============================================================================
// 
//  Project: mm2hack
//  AudioInitializer.h
// 
//  BGM, SE, and channel manager initialization.
// 
//==============================================================================
#pragma once

#include <string>
#include "BgmManager.h"
#include "ChannelManager.h"
#include "SeManager.h"

namespace mm2hack::apps::systems::audio
{
    // AudioInitializer is responsible for initializing audio systems
    class AudioInitializer
    {
    public:
        static bool InitializeAudio(const std::wstring& configPath, BgmManager& bgmManager, SeManager& seManager, ChannelManager& bgmChannels, ChannelManager& seChannels);

    private:
        const std::wstring kClassName{ L"AudioInitializer" };
    };
}