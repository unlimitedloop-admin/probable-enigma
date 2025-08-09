#include "pch.h"

#include "SequenceManager.h"

#include "DebugSequence.h"
#include "SequenceType.h"
#include "StandardSequence.h"
#include "test/TestSequence.h"
#include "utils/output_debug.h"

namespace mm2hack::apps::sequence
{
    void SequenceManager::StartStandardSequence()
    {
        utils::debug_log(L"Start standard sequence.");
        if (_currentSequence)
        {
            StopCurrentSequence();
        }
        _currentSequence = std::make_unique<StandardSequence>();
        _sequenceType = SequenceType::Standard;
    }

    void SequenceManager::StartDebugSequence()
    {
        utils::debug_log(L"Start debug sequence.");
        if (_currentSequence)
        {
            StopCurrentSequence();
        }
        _currentSequence = std::make_unique<DebugSequence>();
        _sequenceType = SequenceType::Debug;
    }

    void SequenceManager::StartTestSequence(const int no)
    {
        utils::debug_log(L"Start test sequence.");
        if (_currentSequence)
        {
            StopCurrentSequence();
        }
        _currentSequence = std::make_unique<TestSequence>(no);
    }

    void SequenceManager::StopCurrentSequence()
    {
        utils::debug_log(L"Stop current sequence.");
        Release();
    }

    void SequenceManager::RebootCurrentSequence()
    {
        utils::debug_log(L"Reboot sequence.");

        switch (_sequenceType)
        {
        case SequenceType::Standard:
            StartStandardSequence();
            break;
        case SequenceType::Debug:
            StartDebugSequence();
            break;
        default:
            utils::debug_log(L"No sequence to reboot.");
            break;
        }
    }

    void SequenceManager::LoadSequence(const SequenceType type)
    {
        utils::debug_log(L"Load sequence from sav file.");

        switch (type)
        {
        case SequenceType::Standard:
            StartStandardSequence();
            break;
        case SequenceType::Debug:
            StartDebugSequence();
            break;
        default:
            break;
        }
    }

    void SequenceManager::Update()
    {
        if (_currentSequence)
        {
            _feedbackOverlay.Update();
            _currentSequence->Execute();
        }
    }

    void SequenceManager::RenderWorld()
    {
        if (_currentSequence)
        {
            _currentSequence->RenderWorld();
        }
    }

    void SequenceManager::RenderOverlay()
    {
        if (_currentSequence)
        {
            _currentSequence->RenderOverlay();
        }
        _feedbackOverlay.Render();
    }

    void SequenceManager::Release()
    {
        if (_currentSequence)
        {
            _currentSequence.reset();
            _sequenceType = SequenceType::None;
        }
    }

    void SequenceManager::SendFeedback(const std::wstring& message)
    {
        _feedbackOverlay.ShowMessage(message, 180);
    }
}