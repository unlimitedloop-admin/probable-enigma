//==============================================================================
// 
//  Project: mm2hack
//  SpriteManager.h
// 
//  SpriteManager class for managing sprite textures using the ITextureObject interface.
// 
//==============================================================================
#pragma once

#include "ITextureObject.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace mm2hack::apps::graphics
{
    // Manages sprite textures and their properties
    class SpriteManager : public ITextureObject
    {
    public:
        // Load a sprite group from a file, identified by its name
        bool Load(const std::wstring& name, const std::wstring& filepath) override;
        // Uses a sprite group by its name, index, and position
        void Use(const std::wstring& name, int index, int x, int y) override;
        // Removes a sprite group by its name, freeing associated resources
        void Remove(const std::wstring& name) override;

        // Sets the division settings for a sprite group, defining how the sprite is divided into tiles
        void SetDivSettings(const std::wstring& name, int tileWidth, int tileHeight, int tilesX, int tilesY);
        // Updates a sprite's palette color using the NES palette and ensures the changes are reflected
        void ReplacePaletteColor(const std::wstring& name, int targetPaletteIndex, int sourcePaletteIndex);
        // Applies a random color filter to the sprite group
        void ApplyRandomColorFilter(const std::wstring& name);
        // Gets the handle for a specific sprite in the group
        int GetSpriteHandle(const std::wstring& name, int index) const;

    private:
        struct DivSettings
        {
            int tileWidth = 0;
            int tileHeight = 0;
            int tilesX = 0;
            int tilesY = 0;
        };

        std::unordered_map<std::wstring, DivSettings> _divSettings;         // Settings for each sprite
        std::unordered_map<std::wstring, std::vector<int>> _spriteHandles;  // Handles for loaded sprites
        std::unordered_map<std::wstring, int> _softImageHandles;            // Handles for loaded soft images

        // Creates sprite graphs from the soft image handles based on the division settings
        bool CreateSpriteGraphs(const std::wstring& name);
    };
}