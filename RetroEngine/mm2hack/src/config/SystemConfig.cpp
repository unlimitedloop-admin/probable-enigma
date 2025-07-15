#include "SystemConfig.h"

#include <string>

namespace mm2hack::config
{
    const std::wstring SystemConfig::kLogFilePath = L"log";
    const std::wstring SystemConfig::kLogFileName = L"mm2hack.log";
    const std::wstring SystemConfig::kDxLibLogFileName = L"dxlib.log";
    const std::wstring SystemConfig::kWindowClassName = L"MM2HackWindowClass";

    const std::wstring SystemConfig::kNESPaletteFilepath = L"assets\\system\\nes_palette.txt";
}