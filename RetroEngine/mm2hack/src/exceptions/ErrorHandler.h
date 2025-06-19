//==============================================================================
// 
//  Project: mm2hack
//  ErrorHandler.h
// 
//  Project wide error handler.
// 
//==============================================================================
#pragma once

#include <cstdio>
#include <cstdlib>
#include <minwinbase.h>
#include <processthreadsapi.h>
#include <string>
#include <sysinfoapi.h>
#include <Windows.h>
#include "ErrorLevel.h"
#include "utils/LogWriter.h"

namespace mm2hack::exceptions
{
    // ErrorHandler class for handling errors and exceptions
    class ErrorHandler final
    {
    public:
        static void Handle(const std::wstring& message, const std::wstring& className, const std::wstring& methodName, ErrorLevel level)
        {
            using namespace utils;

            std::wstring levelStr;
            switch (level)
            {
            case ErrorLevel::Info:        levelStr = L"INFO";       break;
            case ErrorLevel::Warning:     levelStr = L"WARNING";    break;
            case ErrorLevel::Error:       levelStr = L"ERROR";      break;
            case ErrorLevel::FatalError:  levelStr = L"FATALERROR"; break;
            }

            SYSTEMTIME st;
            GetLocalTime(&st);
            wchar_t timeBuf[64];
            swprintf_s(timeBuf, L"%04d-%02d-%02d %02d:%02d:%02d",
                st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

            std::wstring fullMessage =
                L"[" + levelStr + L"] " +
                timeBuf + L" - " +
                className + L"::" + methodName + L" - " + message;

            LogWriter::Write(fullMessage, levelStr);

            // Show message box for errors and fatal errors.
            if (level != ErrorLevel::Info && level != ErrorLevel::Warning)
            {
                MessageBoxW(nullptr, fullMessage.c_str(), L"Application Error", MB_OK | MB_ICONERROR);
            }

            // Exit if fatal.
            if (level == ErrorLevel::FatalError)
            {
                ExitProcess(EXIT_FAILURE);
            }
        }
    };
}