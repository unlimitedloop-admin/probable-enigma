//==============================================================================
// 
//  Project: mm2hack
//  WATCH_MACROS.h
// 
//  Memory watch macros for easy registration and unregistration of watches.
// 
//==============================================================================
#pragma once

#include "apps/runtime/GameContext.h"
#include "IWatchRegistry.h"

#if defined(_DEBUG) || defined(ENABLE_WATCH)
#define WATCH_ADD(name, expr) \
        mm2hack::apps::runtime::GameContext::GetInstance().Watch().Register((name), [&](){ \
            using mm2hack::core::diagnostics::ToWString; \
            return ToWString((expr)); })

#define WATCH_REMOVE(name) \
        mm2hack::apps::runtime::GameContext::GetInstance().Watch().Unregister((name))
#else
#define WATCH_ADD(name, expr)   ((void)0)
#define WATCH_REMOVE(name)      ((void)0)
#endif
