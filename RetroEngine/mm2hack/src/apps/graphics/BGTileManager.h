//==============================================================================
// 
//  Project: mm2hack
//  BGTileManager.h
// 
//  BGTileManager class for managing background tile textures using the ITextureObject interface.
// 
//==============================================================================
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace mm2hack::apps::graphics
{
    // Manages background tile textures and their properties
    class BGTileManager
    {
    public:
        // Load a BG tile group from a file, identified by its name
        bool Load(const std::wstring& name, const std::wstring& filepath);
        // Use a background tile texture at a specific position
        void Use(const std::wstring& name, int index, int x, int y);
        // Remove a background tile texture and its handles
        void Remove(const std::wstring& name);

        // Sets the division settings for a BG tile group, defining how the BG tileset is divided into tiles
        void SetDivSettings(const std::wstring& name, int tileWidth, int tileHeight, int tilesX, int tilesY);
        // Updates a BG tile's palette color using the NES palette and ensures the changes are reflected
        void ReplacePaletteColor(const std::wstring& name, int targetPaletteIndex, int sourcePaletteIndex);

        // Load map data from a file
        void LoadMapData(const std::wstring& mapFile);
        // Draw the map using the specified tileset
        void DrawMap(const std::wstring& tilesetName, int offsetX = 0, int offsetY = 0);
        // Set the attribute for a specific tile
        void SetTileAttribute(uint8_t tileId, uint8_t attr);
        // Get the attribute for a specific tile
        uint8_t GetTileAttribute(int x, int y) const;

    private:
        // Structure that stores division settings for background tile groups
        struct DivSettings
        {
            int tileWidth = 0;
            int tileHeight = 0;
            int tilesX = 0;
            int tilesY = 0;
        };

        std::unordered_map<std::wstring, DivSettings> _divSettings;         // Tile division settings
        std::unordered_map<std::wstring, std::vector<int>> _tileHandles;    // Handles for loaded tiles
        std::unordered_map<std::wstring, int> _softImageHandles;            // Handles for loaded soft images

        std::vector<uint8_t> _tileMap;         // Tile id
        std::vector<uint8_t> _tileAttributes;  // Attributes for each tile

        int _mapWidth = 16;
        int _mapHeight = 15;

        bool CreateBGTileGraphs(const std::wstring& name);
    };
}