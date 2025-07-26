#include "pch.h"

#include "GameContext.h"

#include "apps/supervisor/ResourceManager.h"
#include "input/JoystickManager.h"

namespace mm2hack::apps::deal
{
    void GameContext::Initialize()
    {
        // Set up the resources if necessary.
        _resourceManager = std::make_unique<supervisor::ResourceManager>();
        _joystickManager = std::make_unique<input::JoystickManager>();
    }

    void GameContext::Shutdown()
    {
        // Cleanup resources if necessary.
        _resourceManager.reset();
        _joystickManager.reset();
    }
}