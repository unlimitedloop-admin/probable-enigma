//==============================================================================
// 
//  Project: mm2hack
//  LogWriter.h
// 
//  Write log messages to a file with a timestamp and log level.
// 
//==============================================================================
#pragma once

#include <string>

namespace mm2hack::utils
{
    // Write log messages to a file with a timestamp and log level
    class LogWriter final
    {
    public:
        LogWriter() = delete;
        ~LogWriter() = delete;
        LogWriter(const LogWriter&) = delete;
        LogWriter(LogWriter&&) = delete;
        LogWriter& operator=(const LogWriter&) = delete;
        LogWriter& operator=(LogWriter&&) = delete;
        // This class is not copyable or movable (static class).

        // Create a directory to store the log files
        static void Initialize(const std::wstring& logDirectory);
        // Write a log message to the log file with a timestamp and log level
        static void Write(const std::wstring& message, const std::wstring& level = L"INFO");
        // Close the log file and clean up resources
        static void Shutdown();

    private:
        static std::wstring _logFilePath;
        static bool _initialized;
    };
}