//==============================================================================
// 
//  Project: mm2hack
//  ISequence.h
// 
//  A framework (interface) used to implement game execution modes.
// 
//==============================================================================
#pragma once

#include "apps/scenes/SceneManager.h"
#include "core/save/SaveData.h"

namespace mm2hack::apps::sequence
{
    // Mode setting sequence interface
    class ISequence
    {
    public:
        virtual ~ISequence() = default;
        virtual void Execute() = 0;
        virtual scenes::SceneManager* GetSceneManager() = 0;
        virtual bool Save(core::save::SaveData& out) const = 0;
        virtual bool Load(const core::save::SaveData& in) = 0;
    };
}