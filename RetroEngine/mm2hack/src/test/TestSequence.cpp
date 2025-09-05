#include "pch.h"

#include "TestSequence.h"

#include <stdexcept>
#include "apps/scenes/SceneManager.h"
#include "core/overlay/PauseManager.h"
#include "core/save/SaveData.h"
#include "driver/001/PressKeyCommand.h"
#include "driver/002/DrawGraph.h"
#include "driver/003/SoundTest.h"

namespace mm2hack::apps::sequence
{
    TestSequence::TestSequence(const int scriptNo)
    {
        switch (scriptNo)
        {
        case 1:
            _driver = std::make_unique<scenes::PressKeyCommand>();
            break;
        case 2:
            _driver = std::make_unique<scenes::DrawGraph>();
            break;
        case 3:
            _driver = std::make_unique<scenes::SoundTest>();
            break;
        default:
            throw std::invalid_argument("Invalid script number");
        }

        if (!_driver->Initialize())
        {
            throw std::runtime_error("Failed to initialize the test driver.");
        }
    }

    TestSequence::~TestSequence()
    {
        if (_driver)
        {
            _driver.get()->Finalize();
            _driver.reset();
        }
    }

    void TestSequence::Execute()
    {
        using namespace core::overlay;
        if (!PauseManager::IsPaused())
        {
            _driver.get()->Update();
        }
    }

    void TestSequence::RenderWorld()
    {
        using namespace core::overlay;
        if (_driver)
        {
            _driver.get()->RenderWorld();
        }

        if (PauseManager::IsPaused())
        {
            PauseManager::DrawOverlay();
        }
    }

    void TestSequence::RenderOverlay()
    {
        using namespace core::overlay;
        if (_driver)
        {
            _driver.get()->RenderOverlay();
        }
    }

    scenes::SceneManager* TestSequence::GetSceneManager()
    {
        return nullptr;
    }

    bool TestSequence::Save(core::save::SaveData& out) const
    {
        return true;
    }

    bool TestSequence::Load(const core::save::SaveData& in)
    {
        return true;
    }
}