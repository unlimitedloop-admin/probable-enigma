//==============================================================================
// 
//  Project: mm2hack
//  GameAssets.h
// 
//  This file contains the definitions for the game asset paths used in the project.
// 
//==============================================================================
#pragma once

#include "AssetPaths.h"

namespace mm2hack::config
{
#define X(sym, base, pngRel, jsonRel) \
    MM2H_MAKE_WPATH(sym##SpritePath, base, pngRel); \
    MM2H_MAKE_WPATH(sym##SpriteMeta, base, jsonRel);
#include "AssetsPortfolio.def"
#undef X
}

// "Macros to access the paths"
#define MM2H_GRAPHICS(sym)   (::mm2hack::config::sym##SpritePath)
// "Macros to access the paths"
#define MM2H_GRAPHPROPS(sym) (::mm2hack::config::sym##SpriteMeta)
