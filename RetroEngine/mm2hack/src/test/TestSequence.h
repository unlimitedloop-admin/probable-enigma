//==============================================================================
// 
//  Project: mm2hack
//  TestSequence.h
// 
//  It's only debug space, so it doesn't need to be too complex.
// 
//==============================================================================
#pragma once

#include "apps/sequence/ISequence.h"

#include <memory>
#include "apps/scenes/SceneManager.h"
#include "driver/ITestDriver.h"

namespace mm2hack::apps::sequence
{
    class TestSequence : public ISequence
    {
    public:
        TestSequence(const int scriptNo);
        ~TestSequence() override;
        void Execute() override;
        scenes::SceneManager* GetSceneManager() override;

    private:
        std::unique_ptr<scenes::ITestDriver> _driver;
    };
}