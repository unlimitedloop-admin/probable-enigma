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
#include <string>
#include "driver/ITestDriver.h"

namespace mm2hack::apps::scenes
{
    class SceneManager;
}

namespace mm2hack::core::save
{
    struct SaveData;
}

namespace mm2hack::apps::sequence
{
    class TestSequence : public ISequence
    {
    public:
        TestSequence(const int scriptNo);
        ~TestSequence() override;
        void Execute() override;
        void RenderWorld() override;
        void RenderOverlay() override;
        bool Save(core::save::SaveData& out) const override;
        bool Load(const core::save::SaveData& in) override;

    private:
        const std::wstring kClassName{ L"TestSequence" };

        std::unique_ptr<scenes::ITestDriver> _driver;
    };
}