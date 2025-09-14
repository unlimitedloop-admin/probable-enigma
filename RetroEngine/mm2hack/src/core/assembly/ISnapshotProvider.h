//==============================================================================
// 
//  Project: mm2hack
//  ISnapshotProvider.h
// 
//  Providing game state snapshots. (W.I.P.)
// 
//==============================================================================
#pragma once

namespace mm2hack::core::assembly
{
    // Interface for providing snapshots of the game state at specific points in time
    class ISnapshotProvider
    {
    public:
        virtual ~ISnapshotProvider() = default;
        // Captures the current state of the game and returns a snapshot object
        //virtual GameSnapshot CaptureSnapshot() = 0;
        // Restores the game state from a previously captured snapshot
        //virtual void RestoreSnapshot(const GameSnapshot& snapshot) = 0;

        // Work in progress...
    };
}