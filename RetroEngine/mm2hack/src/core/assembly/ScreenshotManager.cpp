#include "pch.h"

#include "ScreenshotManager.h"

#include <ctime>
#include <filesystem>
#include <iomanip>
#include "core/winapi/WindowManager.h"

namespace fs = std::filesystem;

namespace mm2hack::core::assembly
{
    void ScreenshotManager::CaptureToPng()
    {
        auto& windowManager = mm2hack::core::winapi::WindowManager::GetInstance();
        const int width = windowManager.GetScreenWidth();
        const int height = windowManager.GetScreenHeight();

        fs::path folder = L"./screenshot/";
        if (!fs::exists(folder))
        {
            fs::create_directories(folder);
        }

        auto now = std::chrono::system_clock::now();
        auto t = std::chrono::system_clock::to_time_t(now);
        std::tm local_tm{};
        localtime_s(&local_tm, &t);

        std::wostringstream oss;
        oss << folder.wstring()
            << L"screenshot_"
            << std::put_time(&local_tm, L"%Y-%m-%d_%H-%M-%S")
            << L".png";

        const std::wstring filename = oss.str();

        // Output the displayed window contents to a PNG image in the current window size.
        DxLib::SaveDrawScreenToPNG(0, 0, width - 1, height - 1, filename.c_str());
    }
}