#include "SequenceManager.h"

#include <memory>
#include "DebugSequence.h"
#include "SequenceType.h"
#include "StandardSequence.h"
#include "test/TestSequence.h"
#include "utils/output_debug.h"

namespace mm2hack::apps::sequence
{
    void SequenceManager::StartStandardSequence()
    {
        // Create a new standard sequence
        if (_currentSequence)
        {
            StopCurrentSequence();
        }
        _currentSequence = std::make_unique<StandardSequence>();
        _sequenceType = SequenceType::Standard;
        utils::debug_log(L"Start standard sequence.");
    }

    void SequenceManager::StartDebugSequence()
    {
        // Create a new debug sequence
        if (_currentSequence)
        {
            StopCurrentSequence();
        }
        _currentSequence = std::make_unique<DebugSequence>();
        _sequenceType = SequenceType::Debug;
        utils::debug_log(L"Start debug sequence.");
    }

    void SequenceManager::StartTestSequence(const int no)
    {
        if (_currentSequence)
        {
            StopCurrentSequence();
        }
        _currentSequence = std::make_unique<TestSequence>(no);
        utils::debug_log(L"Start test sequence.");
    }

    void SequenceManager::StopCurrentSequence()
    {
        Release();
        utils::debug_log(L"Drop current sequence.");
    }

    void SequenceManager::RebootCurrentSequence()
    {
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

        utils::debug_log(L"Reboot sequence.");
    }

    void SequenceManager::Update()
    {
        if (_currentSequence)
        {
            _currentSequence->Execute();
        }
    }

    void SequenceManager::Release()
    {
        if (_currentSequence)
        {
            _currentSequence.reset();
            _sequenceType = SequenceType::None;
        }
    }
}