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

        void StartStandardSequence();
        void StartDebugSequence();
        void StopCurrentSequence();
        void RebootCurrentSequence();
        void Update();
        void Release();

        // TODO: Get the current sequence
        //ISequence* GetCurrentSequence() const
        //{
        //    return _currentSequence.get();
        //}

    private:
        SequenceManager() = default;
        ~SequenceManager() = default;

        // TODO: std::unique_ptr<ISequence> _currentSequence = nullptr;
        // HACK: core::overlay::FeedbackOverlay _feedbackOverlay;
    };
}