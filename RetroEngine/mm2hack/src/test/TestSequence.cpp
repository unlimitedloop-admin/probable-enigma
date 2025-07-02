#include "TestSequence.h"

#include <memory>
#include <stdexcept>
#include "apps/scenes/SceneManager.h"
#include "driver/001/PressKeyCommand.h"

namespace mm2hack::apps::sequence
{
    TestSequence::TestSequence(const int scriptNo)
    {
        switch (scriptNo)
        {
        case 1:
            _driver = std::make_unique<scenes::PressKeyCommand>();
            break;
        default:
            throw std::invalid_argument("Invalid script number");
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
        _driver.get()->Update();
    }

    scenes::SceneManager* TestSequence::GetSceneManager()
    {
        return nullptr;
    }
}