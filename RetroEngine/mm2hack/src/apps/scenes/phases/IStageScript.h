//==============================================================================
// 
//  Project: mm2hack
//  IStageScript.h
// 
//  Interface for stage scripts.
// 
//==============================================================================
#pragma once

#include <string>

namespace mm2hack::apps::scenes::phases
{
    struct StageRuntimeContext;

    // Interface for stage scripts
    class IStageScript
    {
    public:
        virtual ~IStageScript() = default;
        // Called when action phase begins (optional)
        virtual void OnEnter(const std::wstring& area_key, StageRuntimeContext& ctx) = 0;
        // Called every frame (optional)
        virtual void OnUpdate(const std::wstring& area_key, StageRuntimeContext& ctx) = 0;
        // Called when action phase ends (optional)
        virtual void OnExit(const std::wstring& area_key, StageRuntimeContext& ctx) = 0;
    };
}