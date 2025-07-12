#include "DrawGraph.h"

#include <DxLib.h>

namespace mm2hack::apps::scenes
{
    bool DrawGraph::Initialize()
    {
        graph_handle = DxLib::LoadGraph(L"src\\resources\\exams\\bg\\default-windowsize-picture.png");
        return graph_handle != -1;
    }

    void DrawGraph::Update()
    {
        if (graph_handle != -1)
        {
            DxLib::DrawGraph(0, 0, graph_handle, TRUE);
        }
    }

    void DrawGraph::Finalize()
    {
        if (graph_handle != -1)
        {
            DxLib::DeleteGraph(graph_handle);
            graph_handle = -1;
        }
    }
}