//==============================================================================
// 
//  Project: mm2hack
//  SaveSystem.h
// 
//  I/O function that handles saving and loading game data.
// 
//==============================================================================
#pragma once

#include <string>
#include "SaveData.h"

namespace mm2hack::core::save
{
    // Provides save and load functionality
    class SaveSystem
    {
    public:
        // Saves the game data to the specified path
        static bool Save(const std::wstring& path, const SaveData& data);
        // Loads the game data from the specified path
        static bool Load(const std::wstring& path, SaveData& outData);
        // Sets the current save slot index
        static void SetCurrentSlot(int slot);
        // Gets the current save slot index
        static int GetCurrentSlot();
        // Get current save slot name
        static std::wstring GetCurrentSlotFilename();

    private:
        static inline int _currentSlot = 0;     // Current save slot index
    };
}