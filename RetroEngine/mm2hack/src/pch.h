#pragma once

// ==========================
// Windows configuration
// ==========================
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

// ==========================
// C++ Standard Library
// ==========================
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <deque>
#include <fstream>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

// ==========================
// DxLib
// ==========================
#include <DxLib.h>

// ==========================
// Common Libraries
// ==========================
#include "config/SystemConfig.h"

// ==========================
// Exception throwing macro
// ==========================
#include "exceptions/CoreException.h"
#include "exceptions/ErrorHandler.h"
#include "exceptions/ErrorLevel.h"
