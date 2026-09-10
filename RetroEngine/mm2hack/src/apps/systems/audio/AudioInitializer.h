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

namespace mm2hack::apps::systems::audio
{
    class BgmManager;
    class SeManager;

    // AudioInitializer is responsible for initializing audio systems
    class AudioInitializer
    {
    public:
        static bool InitializeAudio(
            const std::wstring& configPath,
            BgmManager& bgmManager,
            SeManager& seManager);

    private:
        const std::wstring kClassName{ L"AudioInitializer" };
    };
}
