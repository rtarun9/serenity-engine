#pragma once

// Setup of certain constexpr values.
#ifdef DEF_SERENITY_DEBUG
static constexpr bool SERENITY_DEBUG = true;
#else
static constexpr bool SERENITY_DEBUG = false;
#endif

// STL includes.
#include <array>
#include <filesystem>
#include <format>
#include <memory>
#include <source_location>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using namespace std::string_literals;

// D3D12 includes.
#include <d3d12.h>

// Global project includes.
#include "core/log.hpp"