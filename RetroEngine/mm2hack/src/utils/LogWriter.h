//==============================================================================
// 
//  Project: mm2hack
//  LogWriter.h
// 
//  Output a message to a log file.
// 
//==============================================================================
#pragma once

#include <string>

namespace mm2hack::utils
{
    // Write the specified string, log level and timestamp to a log file
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
        inline static const std::wstring kClassName{ L"LogWriter" };

        static std::wstring _logFilePath;   // Path to the log file
        static bool _initialized;           // Indicates if the logger has been initialized
    };
}