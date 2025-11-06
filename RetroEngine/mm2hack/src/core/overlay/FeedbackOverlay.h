//==============================================================================
// 
//  Project: mm2hack
//  FeedbackOverlay.h
// 
//  It offers a semi-transparent pop-up message overlay.
// 
//==============================================================================
#pragma once

#include <deque>
#include <string>

namespace mm2hack::core::overlay
{
    // Overlay for displaying feedback messages to the player
    class FeedbackOverlay
    {
    public:
        struct Message
        {
            std::wstring text;
            int frame = 0;
            int duration = 90;      // Display for 90 frames by default.
        };

        FeedbackOverlay();
        ~FeedbackOverlay();

        // Show a message on the overlay
        void ShowMessage(const std::wstring& message, int duration = 90);
        // Update the overlay state
        void Update();
        // Render the overlay
        void Render(int destW, int destH);

    private:
        const std::wstring kClassName = L"FeedbackOverlay";

        std::deque<Message> _messages;      // Queue of messages to display
        int _fontScreen = -1;
        int _fontHandle = -1;
    };
}