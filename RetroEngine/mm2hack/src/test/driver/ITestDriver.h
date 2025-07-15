//==============================================================================
// 
//  Project: mm2hack
//  ITestDriver.h
// 
//  It's a test driver; its sole purpose is to check if the features work.
// 
//==============================================================================
#pragma once

namespace mm2hack::apps::scenes
{
    class ITestDriver
    {
    public:
        virtual ~ITestDriver() = default;
        virtual bool Initialize() = 0;
        virtual void Update() = 0;
        virtual void Draw() = 0; // Optional, if drawing is needed
        virtual void Finalize() = 0;
    };
}