//==============================================================================
// 
//  Project: mm2hack
//  GameAssets.h
// 
//  ** Descriptions **
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
#define MM2H_PROPERTIES(sym) (::mm2hack::config::sym##SpriteMeta)
