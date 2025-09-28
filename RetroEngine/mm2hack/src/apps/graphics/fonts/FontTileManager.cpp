#include "pch.h"

#include "FontTileManager.h"

#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string_view>
#include "config/GameAssets.h"
#include "utils/string_converter.h"

namespace mm2hack::apps::graphics::fonts
{
    FontTileManager::~FontTileManager()
    {
        for (auto& [_, set] : _fontSets)
        {
            for (auto& [_, g] : set.graphs) { ::DxLib::DeleteGraph(g); }
            if (set.softImage != -1) { ::DxLib::DeleteSoftImage(set.softImage); }
        }
    }

    void FontTileManager::Load(const std::wstring& name, std::wstring_view jsonPath)
    {
        Load(name, std::wstring_view{}, jsonPath);
    }

    void FontTileManager::Load(const std::wstring& name, std::wstring_view pngPath, std::wstring_view jsonPath)
    {
        Remove(name);
        ParsedMeta meta = ParseMeta_(std::wstring(jsonPath), name);

        if (!pngPath.empty())
        {
            meta.pngPath = std::wstring(pngPath);
        }
        // If PNG path is still empty, derive it from JSON path.
        if (meta.pngPath.empty())
        {
            meta.pngPath = DerivePngFromJsonPath_(std::wstring(jsonPath));
        }

        if (meta.pngPath.empty())
        {
            THROW_EXCEPTION(L"PNG path is not specified", kClassName);
        }

        const int soft = ::DxLib::LoadSoftImage(meta.pngPath.c_str());
        if (soft == -1)
        {
            THROW_EXCEPTION(L"LoadSoftImage failed", kClassName);
        }

        CreateFontGraphs_(name, soft, meta.tile_w, meta.tile_h, meta.tiles_x, meta.tiles_y, meta.charToIndex);
    }

    void FontTileManager::Remove(const std::wstring& name)
    {
        auto it = _fontSets.find(name);
        if (it == _fontSets.end()) return;
        for (auto& [_, g] : it->second.graphs) { ::DxLib::DeleteGraph(g); }
        if (it->second.softImage != -1) { ::DxLib::DeleteSoftImage(it->second.softImage); }
        _fontSets.erase(it);
    }

    void FontTileManager::DrawTextImage(const std::wstring& text, int x, int y, int spacing) const
    {
        int cursorX = x;
        for (wchar_t wc : text)
        {
            char ch = (wc >= 0 && wc <= 0x7F) ? static_cast<char>(wc) : '?';
            if (ch == ' ') { cursorX += spacing; continue; }

            // Draw the first found FontSet.
            for (const auto& [_, set] : _fontSets)
            {
                auto it = set.graphs.find(ch);
                if (it != set.graphs.end())
                {
                    ::DxLib::DrawGraph(cursorX, y, it->second, TRUE);
                    break;
                }
            }
            cursorX += spacing;
        }
    }

    void FontTileManager::ChangeColoredImage(const std::wstring& setName, char ch, uint8_t r, uint8_t g, uint8_t b)
    {
        ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
        auto it = _fontSets.find(setName);
        if (it == _fontSets.end()) return;
        auto& set = it->second;

        auto itIdx = set.charToIndex.find(ch);
        if (itIdx == set.charToIndex.end()) return;
        const int idx = itIdx->second;

        // Palette rewrite
        constexpr int paletteIndex = 1; // Can be changed according to project style.
        ::DxLib::SetPaletteSoftImage(set.softImage, paletteIndex, r, g, b, 255);

        // Re-extract and replace the rectangle of the target tile.
        const int x = (idx % set.tiles_x) * set.tile_w;
        const int y = (idx / set.tiles_x) * set.tile_h;
        const int newGraph = ::DxLib::CreateGraphFromRectSoftImage(set.softImage, x, y, set.tile_w, set.tile_h);
        if (newGraph != -1)
        {
            // Discard old graph.
            if (auto itG = set.graphs.find(ch); itG != set.graphs.end()) { ::DxLib::DeleteGraph(itG->second); }
            set.graphs[ch] = newGraph;
        }
    }

    void FontTileManager::SetUp()
    {
        Load(L"alphabet", MM2H_GRAPHICS(Alphabet), MM2H_PROPERTIES(Alphabet));
        Load(L"numbers", MM2H_GRAPHICS(Numbers), MM2H_PROPERTIES(Numbers));
    }

    void FontTileManager::Shutdown()
    {
        Remove(L"alphabet");
        Remove(L"numbers");
    }

    // --- helpers ---
    FontTileManager::ParsedMeta FontTileManager::ParseMeta_(const std::wstring& jsonPath, const std::wstring& setName)
    {
        // --- JSON meta (supports both top-level and "Loader" wrapper) ---
        ParsedMeta out{};
        std::filesystem::path jpath(jsonPath);

        bool jsonLoaded = false;
        std::ifstream ifs(jpath);
        if (ifs)
        {
            nlohmann::json j; ifs >> j; jsonLoaded = true;
            const auto& root = j;

            const nlohmann::json* loader = nullptr;
            if (auto it = root.find("Loader"); it != root.end() && it->is_object()) loader = &(*it);

            auto getv = [&](const char* key, int& dst)
                {
                    if (loader && loader->contains(key)) dst = (*loader)[key].get<int>();
                    else if (root.contains(key)) dst = root[key].get<int>();
                };

            getv("tileWidth", out.tile_w);
            getv("tileHeight", out.tile_h);
            getv("tilesX", out.tiles_x);
            getv("tilesY", out.tiles_y);

            // If not specified, defaults remain (8x8, 16x2).
            auto get_png = [&](const nlohmann::json& obj)->std::wstring
                {
                    if (auto it = obj.find("png"); it != obj.end() && it->is_string())
                        return utils::utf8_to_wstring(it->get<std::string>());
                    return L"";
                };
            std::wstring pngW;
            if (loader) pngW = get_png(*loader);
            if (pngW.empty()) pngW = get_png(root);
            if (!pngW.empty()) out.pngPath = ConcatPath_(jpath.parent_path(), pngW);

            // char -> index map
            if (auto it = root.find("map"); it != root.end() && it->is_object())
            {
                for (auto it2 = it->begin(); it2 != it->end(); ++it2)
                {
                    if (!it2.value().is_number_integer()) continue;
                    const std::string& k = it2.key(); if (k.empty()) continue;
                    const char ch = static_cast<char>(std::toupper(static_cast<unsigned char>(k[0])));
                    out.charToIndex[ch] = it2.value().get<int>();
                }
            }
        }

        // --- Fill defaults if necessary ---
        if (out.charToIndex.empty())
        {
            if (setName == L"alphabet")
            {
                for (int i = 0; i < 26; ++i) out.charToIndex['A' + i] = i;
                out.charToIndex['r'] = 26; out.charToIndex['.'] = 27; out.charToIndex[','] = 28;
                out.charToIndex['\''] = 29; out.charToIndex['!'] = 30; out.charToIndex['?'] = 31;
            }
            else if (setName == L"numbers")
            {
                for (int i = 0; i < 10; ++i) out.charToIndex['0' + i] = i;
                if (!jsonLoaded) { out.tiles_x = 10; out.tiles_y = 1; }
            }
        }

        // Default tile size if not loaded from JSON.
        if (!jsonLoaded)
        {
            if (setName == L"alphabet") { out.tile_w = 8; out.tile_h = 8; out.tiles_x = 16; out.tiles_y = 2; }
            else if (setName == L"numbers") { out.tile_w = 8; out.tile_h = 8; out.tiles_x = 10; out.tiles_y = 1; }
        }

        if (out.pngPath.empty()) out.pngPath = DerivePngFromJsonPath_(jsonPath);

        return out;
    }

    std::wstring FontTileManager::DerivePngFromJsonPath_(const std::wstring& jsonPath)
    {
        std::filesystem::path p(jsonPath);
        p.replace_extension(L".png");
        return p.wstring();
    }

    std::wstring FontTileManager::ConcatPath_(const std::filesystem::path& baseDir, const std::wstring& rel)
    {
        if (rel.empty()) return baseDir.wstring();
        std::filesystem::path r(rel);
        if (r.is_absolute()) return r.wstring();
        std::filesystem::path joined = baseDir / r;
        return joined.wstring();
    }

    void FontTileManager::CreateFontGraphs_(const std::wstring& name, int softImage,
        int tile_w, int tile_h, int tiles_x, int tiles_y,
        const std::map<char, int>& charIndexMap)
    {
        const int total = tiles_x * tiles_y;
        std::vector<int> handles(static_cast<std::size_t>(total), -1);
        const int res = ::DxLib::CreateDivGraphFromSoftImage(softImage, total, tiles_x, tiles_y, tile_w, tile_h, handles.data());
        if (res != 0)
        {
            ::DxLib::DeleteSoftImage(softImage);
            THROW_EXCEPTION(L"CreateDivGraphFromSoftImage failed", kClassName);
        }

        FontSet set;
        set.softImage = softImage;
        set.tile_w = tile_w; set.tile_h = tile_h; set.tiles_x = tiles_x; set.tiles_y = tiles_y;

        for (const auto& [ch, idx] : charIndexMap)
        {
            if (idx >= 0 && idx < total)
            {
                set.graphs[ch] = handles[static_cast<std::size_t>(idx)];
                set.charToIndex[ch] = idx;
            }
        }

        _fontSets[name] = std::move(set);
    }
}