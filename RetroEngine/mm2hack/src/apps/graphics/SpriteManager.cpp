#include "pch.h"

#include "SpriteManager.h"

#include <cstdlib>
#include <utility>
#include "apps/NES/NESPalette.h"
#include "exceptions/CoreException.h"
#include "utils/string_converter.h"

namespace mm2hack::apps::graphics
{
    bool SpriteManager::Load(const std::wstring& name, const std::wstring& filepath)
    {
        // Check if the loading parameters of sprite are set for specified name (If not, an error will occur).
        auto it = _divSettings.find(name);
        if (it == _divSettings.end())
        {
            THROW_EXCEPTION("Div settings not set for sprite: " + utils::wstring_to_utf8(name), L"SpriteManager");
        }

        int soft = DxLib::LoadSoftImage(filepath.c_str());
        if (soft == -1)
        {
            THROW_EXCEPTION("Failed to load sprite texture: " + utils::wstring_to_utf8(filepath), L"SpriteManager");
        }
        _softImageHandles[name] = soft;

        return CreateSpriteGraphs(name);
    }

    void SpriteManager::Use(const std::wstring& name, int index, int x, int y)
    {
        // Check if the sprite is loaded and the index is valid
        auto it = _spriteHandles.find(name);
        if (it != _spriteHandles.end() && index >= 0 && index < static_cast<int>(it->second.size()))
        {
            DxLib::DrawGraph(x, y, it->second[index], TRUE);
        }
    }

    void SpriteManager::Remove(const std::wstring& name)
    {
        auto it = _spriteHandles.find(name);
        if (it != _spriteHandles.end())
        {
            for (int handle : it->second)
            {
                DxLib::DeleteGraph(handle);
            }
            _spriteHandles.erase(it);
        }

        auto softIt = _softImageHandles.find(name);
        if (softIt != _softImageHandles.end())
        {
            DxLib::DeleteSoftImage(softIt->second);
            _softImageHandles.erase(softIt);
        }
    }

    void SpriteManager::SetDivSettings(const std::wstring& name, int tileWidth, int tileHeight, int tilesX, int tilesY)
    {
        // Set various parameters used when loading graphics in advance
        // For example, tile width, height, number of tiles in the X and Y directions, etc.
        _divSettings[name] = { tileWidth, tileHeight, tilesX, tilesY };
    }

    void SpriteManager::ReplacePaletteColor(const std::wstring& name, int targetPaletteIndex, int sourcePaletteIndex)
    {
        using NES::NESPalette;
        auto it = _softImageHandles.find(name);
        if (it == _softImageHandles.end())
        {
            THROW_EXCEPTION("Soft image not found for sprite: " + utils::wstring_to_utf8(name), L"SpriteManager");
        }

        const auto& rgb = NESPalette::GetColor(targetPaletteIndex);
        if (DxLib::SetPaletteSoftImage(it->second, sourcePaletteIndex, rgb.red, rgb.green, rgb.blue, 255) != 0)
        {
            THROW_EXCEPTION("Failed to set palette for sprite: " + utils::wstring_to_utf8(name), L"SpriteManager");
        }

        if (!CreateSpriteGraphs(name))
        {
            THROW_EXCEPTION("Failed to rebuild sprite graphs after palette change: " + utils::wstring_to_utf8(name), L"SpriteManager");
        }
    }

    void SpriteManager::ApplyRandomColorFilter(const std::wstring& name)
    {
        auto it = _spriteHandles.find(name);
        if (it == _spriteHandles.end()) return;

        // -128 to 127
        int r = rand() % 0x100 - 0x80;

        for (int handle : it->second)
        {
            DxLib::GraphFilter(handle, DX_GRAPH_FILTER_HSB, 0, r, 0, 0);    // Apply a random color filter that only the hue is changed.
        }
    }

    int SpriteManager::GetSpriteHandle(const std::wstring& name, int index) const
    {
        auto it = _spriteHandles.find(name);
        if (it != _spriteHandles.end() && index >= 0 && index < static_cast<int>(it->second.size()))
        {
            return it->second[index];
        }
        return -1;      // Invalid handle
    }

    bool SpriteManager::CreateSpriteGraphs(const std::wstring& name)
    {
        auto itSetting = _divSettings.find(name);
        auto itSoft = _softImageHandles.find(name);
        if (itSetting == _divSettings.end() || itSoft == _softImageHandles.end())
        {
            return false;       // Not found.
        }

        const DivSettings& settings = itSetting->second;
        int soft = itSoft->second;
        int totalCount = settings.tilesX * settings.tilesY;
        std::vector<int> handles(totalCount);
        if (DxLib::CreateDivGraphFromSoftImage(soft, totalCount, settings.tilesX, settings.tilesY,
            settings.tileWidth, settings.tileHeight, handles.data()) != 0)
        {
            return false;       // Failed to create div graph.
        }

        _spriteHandles[name] = std::move(handles);
        return true;
    }
}