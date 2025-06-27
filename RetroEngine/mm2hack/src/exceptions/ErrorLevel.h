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
    // The values are defined as follows:
    // 0 - No error
    // 1 - Warning
    // 2 - Error
    // 3 - Fatal error
    enum class ErrorLevel
    {
        Info,
        Warning,
        Error,
        FatalError
    };
}