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
#include "core/save/SaveData.h"
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
        bool Save(core::save::SaveData& out) const override;
        bool Load(const core::save::SaveData& in) override;

    private:
        std::unique_ptr<scenes::ITestDriver> _driver;
    };
}