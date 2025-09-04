//==============================================================================
// 
//  Project: mm2hack
//  CoreException.h
// 
//  Exception class for handling errors in the application.
// 
//==============================================================================
#pragma once

#include <cstring>
#include <stdexcept>
#include <string>
#include "ErrorLevel.h"
#include "utils/string_converter.h"

namespace mm2hack::exceptions
{
    // Exception class with additional information (class name and method name)
    class CoreException : public std::runtime_error
    {
    public:
        // Error message, class name, and method name
        CoreException(
            const std::string& message,
            const std::wstring& className,
            const char* methodName,
            const ErrorLevel& errorLevel = ErrorLevel::Error,
            const char* file = "",
            int line = 0
        )
            : std::runtime_error(message),
            _className(className),
            _methodName(methodName ? std::wstring(methodName, methodName + strlen(methodName)) : L""),
            _errorLevel(errorLevel),
            _file(file),
            _line(line)
        {
        }

        // Wide string constructor
        CoreException(
            const std::wstring& message,
            const std::wstring& className,
            const char* methodName,
            const ErrorLevel& errorLevel = ErrorLevel::Error,
            const char* file = "",
            int line = 0
        )
            : std::runtime_error(utils::wstring_to_utf8(message)),
            _className(className),
            _methodName(methodName ? std::wstring(methodName, methodName + strlen(methodName)) : L""),
            _errorLevel(errorLevel),
            _file(file),
            _line(line)
        {
        }


        const std::wstring& GetKlassName() const { return _className; }
        const std::wstring& GetMethodName() const { return _methodName; }
        const ErrorLevel& GetErrorLevel() const { return _errorLevel; }
        const char* GetFile() const { return _file; }
        int GetLine() const { return _line; }

    private:
        std::wstring _className;
        std::wstring _methodName;
        ErrorLevel _errorLevel = ErrorLevel::Error; // Default error level
        const char* _file = "";
        int _line = 0;
    };


#define THROW_EXCEPTION(msg, klass) \
        throw mm2hack::exceptions::CoreException((msg), (klass), __func__, mm2hack::exceptions::ErrorLevel::Error, __FILE__, __LINE__)

// Macro to throw an exception with class and method information
#define THROW_EXCEPTION_EX(msg, klass, errlvl) \
        throw mm2hack::exceptions::CoreException((msg), (klass), __func__, (errlvl), __FILE__, __LINE__)

}