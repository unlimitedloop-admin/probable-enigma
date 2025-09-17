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
        // Load a texture from a file and associate it with a name
        virtual bool Load(const std::wstring& name, const std::wstring& filepath) = 0;
        // Use the texture associated with the name at the specified index and coordinates
        virtual void Use(const std::wstring& name, int index, int x, int y) = 0;
        // Remove the texture associated with the name from memory
        virtual void Remove(const std::wstring& name) = 0;
    };
}