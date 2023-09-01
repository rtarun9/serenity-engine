#pragma once

// Setup of certain constexpr values.
#ifdef DEF_SERENITY_DEBUG
static constexpr bool SERENITY_DEBUG = true;
#else
static constexpr bool SERENITY_DEBUG = false;
#endif

// STL includes.
#include <algorithm>
#include <array>
#include <chrono>
#include <exception>
#include <filesystem>
#include <format>
#include <iostream>
#include <memory>
#include <optional>
#include <source_location>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeinfo>
#include <vector>

using namespace std::string_literals;

// D3D12 / Windows includes.
#include "graphics/d3dx12.hpp"
#include <DirectXMath.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace math = DirectX;

#include <wrl.h>

// Global project includes.
#include "core/log.hpp"
#include "utils/primitive_datatypes.hpp"