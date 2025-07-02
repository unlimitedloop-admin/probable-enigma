//==============================================================================
// 
//  Project: mm2hack
//  JoystickSingleton.h
// 
//  Joystick input management singleton.
// 
//==============================================================================
#pragma once

#include <memory>
#include "JoystickManager.h"

namespace mm2hack::input
{
    // Singleton class to manage joystick input
    class JoystickSingleton
    {
    public:
        JoystickSingleton(const JoystickSingleton&) = delete;
        JoystickSingleton& operator=(const JoystickSingleton&) = delete;
        JoystickSingleton(JoystickSingleton&&) = delete;
        JoystickSingleton& operator=(JoystickSingleton&&) = delete;
        JoystickSingleton() = default;
        ~JoystickSingleton() = default;
        // JoystickManager is a singleton, so we prevent copying and moving.

        static JoystickManager& Instance();
        static void Initialize();
        static void Finalize();

    private:
        static std::unique_ptr<JoystickManager> _instance;
    };
}