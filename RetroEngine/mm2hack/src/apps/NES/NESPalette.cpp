#include "NESPalette.h"

#include <DxLib.h>
#include <fstream>
#include <sstream>
#include <string>

namespace mm2hack::apps::NES
{
    bool NESPalette::LoadPaletteFromFile(const std::wstring& file_path)
    {
        std::wifstream file(file_path);
        if (!file.is_open())
        {
            return false;
        }

        std::wstring line;
        while (std::getline(file, line))
        {
            // Skip comments and empty lines
            if (line.empty() || line[0] == L'#')
            {
                continue;
            }

            std::wstringstream ss(line);
            int index, red, green, blue;
            if (ss >> index >> red >> green >> blue)
            {
                if (index >= 0 && index < 64)
                {
                    _palette_data[index] = RGB{ red, green, blue };
                }
            }
        }

        return true;
    }

    void NESPalette::SetBackgroundFor(size_t palette_no)
    {
        if (palette_no >= _palette_data.size())
        {
            palette_no = 0;
        }

        const RGB& color = _palette_data[palette_no];
        DxLib::SetBackgroundColor(color.red, color.green, color.blue);
    }

    // Get the color from external palette text file (0-63)
    const NESPalette::RGB& NESPalette::GetColor(size_t index)
    {
        if (index >= _palette_data.size())
        {
            index = 0;
        }
        return _palette_data.at(index);
    }
}