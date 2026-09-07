//==============================================================================
//
//  Project: mm2hack
//  SaveStateTests.h
//
//  Headless regression tests for save-state validation and continuation.
//
//==============================================================================
#pragma once

namespace mm2hack::test
{
    // Runs without initializing the window, renderer, audio, or game resources.
    int RunSaveStateTests() noexcept;
}
