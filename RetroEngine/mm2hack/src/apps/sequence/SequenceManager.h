//==============================================================================
// 
//  Project: mm2hack
//  SequenceManager.h
// 
//  A class that holds the game's launch mode.
// 
//==============================================================================
#pragma once

#include <memory>
#include <string>
#include "core/overlay/FeedbackOverlay.h"
#include "ISequence.h"
#include "SequenceType.h"

namespace mm2hack::apps::sequence
{
    // SequenceManager is a singleton class that manages the current sequence
    class SequenceManager final
    {
    public:
        static SequenceManager& GetInstance()
        {
            static SequenceManager instance;
            return instance;
        }

        SequenceManager(const SequenceManager&) = delete;
        SequenceManager& operator=(const SequenceManager&) = delete;
        SequenceManager(SequenceManager&&) = delete;
        SequenceManager& operator=(SequenceManager&&) = delete;
        // SequenceManager is a singleton, so we delete the copy and move constructors and assignment operators.

        // Starts the standard sequence, which is the default game mode
        void StartStandardSequence();
        void StartDebugSequence();
        void StartTestSequence(const int no);
        // Ends the current sequence, cleaning up resources
        void StopCurrentSequence();
        // Reboots the current sequence, resetting it to its initial state
        void RebootCurrentSequence();
        // Loads a specific sequence type, such as standard or debug, using when the loading save data
        void LoadSequence(const SequenceType type);
        // Executes the current sequence, which is responsible for running the game logic
        void Update();
        // Renders the graphics for the current sequence
        void RenderWorld();
        // Renders the overlay for the current sequence, typically used for UI elements
        void RenderOverlay(float viewerRate);
        // Releases the current sequence, cleaning up resources
        void Release();

        // Gets the current sequence object, which can be used to access sequence-specific methods
        ISequence* GetCurrentSequence() const
        {
            return _currentSequence.get();
        }

        // Gets the current sequence type, which indicates the mode of the game (e.g., standard, debug)
        SequenceType GetCurrentSequenceType() const
        {
            return _sequenceType;
        }

        // Sends feedback to the user, typically used for displaying messages or notifications
        void SendFeedback(const std::wstring& message);

        // Handles the configuration mode for joystick button mapping
        void HandleJpbtnConfigMode(double dt);

    private:
        SequenceManager() = default;
        ~SequenceManager() = default;

        std::unique_ptr<ISequence> _currentSequence = nullptr;              // Pointer to the current sequence object
        SequenceType _sequenceType = SequenceType::None;                    // Current sequence type, indicating the mode of the game
        core::overlay::FeedbackOverlay _feedbackOverlay;                    // Overlay for displaying feedback messages to the user
    };
}