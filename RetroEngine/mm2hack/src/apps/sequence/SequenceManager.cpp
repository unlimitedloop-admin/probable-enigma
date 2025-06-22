#include "SequenceManager.h"

#include "utils/output_debug.h"

namespace mm2hack::apps::sequence
{
    void SequenceManager::StartStandardSequence()
    {
        utils::debug_log(L"Start standard sequence.");
    }

    void SequenceManager::StartDebugSequence()
    {
        utils::debug_log(L"Start debug sequence.");
    }

    void SequenceManager::StopCurrentSequence()
    {
        Release();
        utils::debug_log(L"Drop current sequence.");
    }

    void SequenceManager::RebootCurrentSequence()
    {
        StopCurrentSequence();
        // TODO: restart current seq. It is necessary to reference a sequence instance.
        utils::debug_log(L"Start standard sequence.");
    }

    void SequenceManager::Update()
    {

    }

    void SequenceManager::Release()
    {

    }
}