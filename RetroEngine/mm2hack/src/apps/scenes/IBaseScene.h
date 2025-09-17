//==============================================================================
// 
//  Project: mm2hack
//  IBaseScene.h
// 
//  A generic interface for scene managing the main game program on-the-fly.
// 
//==============================================================================
#pragma once

#include <string>
#include "apps/parameters/Parameters.h"
#include "SceneID.h"

namespace mm2hack::apps::scenes
{
    // Interface for scene management in the sequence
    class IBaseScene
    {
    public:
        virtual ~IBaseScene() = default;
        
        // === Lifecycle ===
        virtual void Initialize(const parameters::Parameters& params) = 0;
        virtual void Finalize() = 0;

        // === Logic only ===
        virtual void Update() = 0;

        // === Rendering split ===
        // Draw to native render target (will be scaled later).
        virtual void RenderWorld() = 0;
        // Draw to back buffer as overlay (not scaled).
        virtual void RenderOverlay() = 0;
        
        // === Scene identification ===
        virtual SceneID GetSceneID() const = 0;
        virtual std::wstring GetSceneName() const = 0;
    };
}