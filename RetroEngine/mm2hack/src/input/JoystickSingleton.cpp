#include "JoystickSingleton.h"

#include <memory>
#include "JoystickManager.h"

namespace mm2hack::input
{
    std::unique_ptr<JoystickManager> JoystickSingleton::_instance = nullptr;

    void JoystickSingleton::Initialize()
    {
        if (!_instance)
        {
            _instance = std::make_unique<JoystickManager>();
        }
    }

    void JoystickSingleton::Finalize()
    {
        _instance.reset();
    }

    JoystickManager& JoystickSingleton::Instance()
    {
        if (!_instance)
        {
            // It's a failsafe initialization.
            Initialize();
        }
        return *_instance;
    }
}