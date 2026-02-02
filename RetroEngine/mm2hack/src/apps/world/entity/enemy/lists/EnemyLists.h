//==============================================================================
// 
//  Project: mm2hack
//  EnemyLists.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include <cstdint>

namespace mm2hack::apps::world::entity::enemy
{
    // Enemy list for each stage
    enum class EnemyKind : std::uint16_t
    {
        Met,
        SniperJoe,
        BigEye,
        Batton,
        FlyBoy,
    };
}