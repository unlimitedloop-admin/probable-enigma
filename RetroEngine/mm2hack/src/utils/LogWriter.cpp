#include "LogWriter.h"

#include <chrono>
#include <corecrt.h>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <ios>
#include <string>
#include "config/SystemConfig.h"
#include "string_converter.h"

namespace fs = std::filesystem;

namespace mm2hack::utils
{
    std::wstring LogWriter::_logFilePath;
    bool LogWriter::_initialized = false;

    void LogWriter::Initialize(const std::wstring& logDirectory)
    {
        if (!_initialized)
        {
            fs::create_directories(logDirectory);
            _logFilePath = logDirectory + L"\\" + config::SystemConfig::kLogFileName;
            _initialized = true;
        }
    }

    void LogWriter::Write(const std::wstring& message, const std::wstring& level)
    {
        if (!_initialized)
        {
            Initialize(config::SystemConfig::kLogFilePath);
        }

        const std::string utf8Message = wstring_to_utf8(message);
        const std::string utf8Level = wstring_to_utf8(level);

        std::ofstream ofs(_logFilePath, std::ios::app | std::ios::binary);
        if (!ofs.is_open()) return;

        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm local_tm;
        localtime_s(&local_tm, &now_time);

        char timeBuf[64];
        std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", &local_tm);

        std::string finalLog =
            "[" + std::string(timeBuf) + "][" + utf8Level + "] " + utf8Message + "\n";

        ofs.write(finalLog.c_str(), static_cast<std::streamsize>(finalLog.size()));
    }

    void LogWriter::Shutdown()
    {
        // No specific shutdown actions needed for this implementation
    }
}