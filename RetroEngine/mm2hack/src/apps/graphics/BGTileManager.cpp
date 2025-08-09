#include "pch.h"

#include "BGTileManager.h"

#include <cstdint>
#include <fstream>
#include <iterator>
#include <utility>
#include "apps/NES/NESPalette.h"
#include "exceptions/CoreException.h"
#include "utils/string_converter.h"

namespace mm2hack::apps::graphics
{
    bool BGTileManager::Load(const std::wstring& name, const std::wstring& filepath)
    {
        // Check if the loading parameters of tile are set for specified name (If not, an error will occur)
        auto it = _divSettings.find(name);
        if (it == _divSettings.end())
        {
            THROW_EXCEPTION("Div settings not set for BG tile: " + utils::wstring_to_utf8(name), L"BGTileManager");
        }

        int soft = DxLib::LoadSoftImage(filepath.c_str());
        if (soft == -1)
        {
            THROW_EXCEPTION("Failed to load BG tile image: " + utils::wstring_to_utf8(filepath), L"BGTileManager");
        }

        _softImageHandles[name] = soft;
        if (!CreateBGTileGraphs(name))
        {
            DxLib::DeleteSoftImage(soft);
            _softImageHandles.erase(name);
            THROW_EXCEPTION("Failed to create divided BG graph from soft image: " + utils::wstring_to_utf8(filepath), L"BGTileManager");
        }

        return true;
    }

    void BGTileManager::Use(const std::wstring& name, int index, int x, int y)
    {
        // Check if the tile is loaded and the index is valid
        auto it = _tileHandles.find(name);
        if (it != _tileHandles.end() && index >= 0 && index < static_cast<int>(it->second.size()))
        {
            DxLib::DrawGraph(x, y, it->second[index], TRUE);
        }
    }

    void BGTileManager::Remove(const std::wstring& name)
    {
        auto it = _tileHandles.find(name);
        if (it != _tileHandles.end())
        {
            for (int handle : it->second)
            {
                DxLib::DeleteGraph(handle);
            }
            _tileHandles.erase(it);
        }

        auto softIt = _softImageHandles.find(name);
        if (softIt != _softImageHandles.end())
        {
            DxLib::DeleteSoftImage(softIt->second);
            _softImageHandles.erase(softIt);
        }
    }

    void BGTileManager::SetDivSettings(const std::wstring& name, int tileWidth, int tileHeight, int tilesX, int tilesY)
    {
        _divSettings[name] = { tileWidth, tileHeight, tilesX, tilesY };
    }

    void BGTileManager::ReplacePaletteColor(const std::wstring& name, int targetPaletteIndex, int sourcePaletteIndex)
    {
        using namespace NES;
        auto it = _softImageHandles.find(name);
        if (it == _softImageHandles.end())
        {
            THROW_EXCEPTION("Soft image not found for BG tile: " + utils::wstring_to_utf8(name), L"BGTileManager");
        }

        const auto& rgb = NESPalette::GetColor(targetPaletteIndex);
        if (DxLib::SetPaletteSoftImage(it->second, sourcePaletteIndex, rgb.red, rgb.green, rgb.blue, 255) != 0)
        {
            THROW_EXCEPTION("Failed to set palette for BG tile: " + utils::wstring_to_utf8(name), L"BGTileManager");
        }

        if (!CreateBGTileGraphs(name))
        {
            THROW_EXCEPTION("Failed to rebuild BG tile graphs after palette change: " + utils::wstring_to_utf8(name), L"BGTileManager");
        }
    }

    void BGTileManager::LoadMapData(const std::wstring& mapFile)
    {
        std::ifstream file(mapFile, std::ios::binary);
        if (!file)
        {
            THROW_EXCEPTION("Failed to open map file: " + utils::wstring_to_utf8(mapFile), L"BGTileManager");
        }

        file.unsetf(std::ios::skipws);
        std::vector<uint8_t> rawData(std::istream_iterator<uint8_t>{file}, {});
        if (rawData.size() < 0x100)
        {
            THROW_EXCEPTION("Map file is too small: " + utils::wstring_to_utf8(mapFile), L"BGTileManager");
        }

        _tileMap.assign(rawData.begin() + 0x10, rawData.begin() + 0x10 + (_mapWidth * _mapHeight));
    }

    void BGTileManager::DrawMap(const std::wstring& tilesetName, int offsetX, int offsetY)
    {
        // Check if the tileset is loaded
        auto it = _tileHandles.find(tilesetName);
        if (it == _tileHandles.end()) return;

        const int tileXSize = config::SystemConfig::kTileSizeWidth;
        const int tileYSize = config::SystemConfig::kTileSizeHeight;

        const auto& handles = it->second;
        for (int y = 0; y < _mapHeight; ++y)
        {
            for (int x = 0; x < _mapWidth; ++x)
            {
                int index = y * _mapWidth + x;
                uint8_t tileId = _tileMap[index];
                if (tileId < handles.size())
                {
                    // Render the tile at the calculated position using its associated graphic handle.
                    DxLib::DrawGraph(x * tileXSize + offsetX, y * tileYSize + offsetY, handles[tileId], TRUE);
                }
            }
        }
    }

    void BGTileManager::SetTileAttribute(uint8_t tileId, uint8_t attr)
    {
        if (_tileAttributes.size() <= tileId)
        {
            _tileAttributes.resize(tileId + 1);
        }
        _tileAttributes[tileId] = attr;
    }

    uint8_t BGTileManager::GetTileAttribute(int x, int y) const
    {
        if (x < 0 || x >= _mapWidth || y < 0 || y >= _mapHeight) return 0;
        uint8_t tileId = _tileMap[y * _mapWidth + x];
        if (tileId < _tileAttributes.size())
        {
            return _tileAttributes[tileId];
        }
        return 0;
    }

    bool BGTileManager::CreateBGTileGraphs(const std::wstring& name)
    {
        auto itSetting = _divSettings.find(name);
        auto itSoft = _softImageHandles.find(name);
        if (itSetting == _divSettings.end() || itSoft == _softImageHandles.end())
        {
            return false;   // Not found.
        }

        const DivSettings& settings = itSetting->second;
        int soft = itSoft->second;
        int totalCount = settings.tilesX * settings.tilesY;
        std::vector<int> handles(totalCount);
        if (DxLib::CreateDivGraphFromSoftImage(soft, totalCount, settings.tilesX, settings.tilesY,
            settings.tileWidth, settings.tileHeight, handles.data()) != 0)
        {
            return false;   // Failed to create div graph.
        }

        _tileHandles[name] = std::move(handles);
        return true;
    }
}