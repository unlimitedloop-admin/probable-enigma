//==============================================================================
// 
//  Project: mm2hack
//  PathDefsJsonProps.h
// 
//  This file contains the definitions for the JSON property paths used in the project.
// 
//==============================================================================
#pragma once

#include "AssetPaths.h"

namespace mm2hack::config
{
#define X(sym, base, jsonRel) \
    MM2H_MAKE_WPATH(sym##PropertyPath, base, jsonRel);
#include "PropertiesPortfolio.def"
#undef X
}

// "Macros to access the paths"
#define MM2H_PROPERTY(sym) (::mm2hack::config::sym##PropertyPath)
