//==============================================================================
// 
//  Project: mm2hack
//  ITextureObject.h
// 
//  A common interface for loading and unloading graphics resource.
// 
//==============================================================================
#pragma once

#include <string>

namespace mm2hack::apps::graphics
{
    // Interface for texture object management
    class ITextureObject
    {
    public:
        virtual ~ITextureObject() = default;
        virtual bool Load(const std::wstring& name, const std::wstring& filepath) = 0;
        virtual void Use(const std::wstring& name, int index, int x, int y) = 0;
        virtual void Remove(const std::wstring& name) = 0;
    };
}