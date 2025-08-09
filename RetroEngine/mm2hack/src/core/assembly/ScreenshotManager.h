//==============================================================================
// 
//  Project: mm2hack
//  ScreenshotManager.h
// 
//  Ability to save screenshots in picture format.
// 
//==============================================================================
#pragma once

namespace mm2hack::core::assembly
{
    // A class to manage screenshot functionality
    class ScreenshotManager final
    {
    public:
        ScreenshotManager() = delete;
        ~ScreenshotManager() = delete;
        ScreenshotManager(const ScreenshotManager&) = delete;
        ScreenshotManager& operator=(const ScreenshotManager&) = delete;
        ScreenshotManager(ScreenshotManager&&) = delete;
        ScreenshotManager& operator=(ScreenshotManager&&) = delete;
        // This class is not copyable or movable (static class)

        // Capture the current screen and save it as a PNG file
        static void CaptureToPng();
    };
}