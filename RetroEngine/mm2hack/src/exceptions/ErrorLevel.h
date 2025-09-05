//==============================================================================
// 
//  Project: mm2hack
//  ErrorLevel.h
// 
//  ErrorLevels enumeration.
// 
//==============================================================================
#pragma once

namespace mm2hack::exceptions
{
    // Error levels for the application
    // These levels are used to indicate the severity of an error or warning
    enum class ErrorLevel
    {
        Info,           // 0 - No error
        Warning,        // 1 - Warning
        Error,          // 2 - Error
        FatalError      // 3 - Fatal error
    };
}