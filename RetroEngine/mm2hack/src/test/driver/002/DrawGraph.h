//==============================================================================
// 
//  Project: mm2hack
//  DrawGraph.h
// 
//  This is used in window and graphic size scaling tests.
// 
//==============================================================================
#pragma once

#include "test/driver/ITestDriver.h"

namespace mm2hack::apps::scenes
{
    class DrawGraph final : public ITestDriver
    {
    public:
        DrawGraph() {}
        ~DrawGraph() {}

        bool Initialize() override;
        void Update() override;
        void Finalize() override;

    private:
        int graph_handle = -1;  // Handle for the loaded graph
    };
}