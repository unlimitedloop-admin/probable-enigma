#include "pch.h"

#include "OuterSpaceBG.h"

namespace mm2hack::apps::scenes
{
    bool OuterSpaceBG::Initialize()
    {
        _starField.InitStars();
        return true;
    }
    void OuterSpaceBG::Update()
    {
        _starField.UpdateStars();
    }
    void OuterSpaceBG::RenderWorld()
    {
        _starField.DrawStars();
    }
    void OuterSpaceBG::RenderOverlay()
    {
        // No overlay rendering needed
    }
    void OuterSpaceBG::Finalize()
    {
        // No special finalization needed
    }
}