#pragma once

// Setup of certain constexpr values and defines based on build config.
#ifdef DEF_SERENITY_DEBUG
static constexpr bool SERENITY_DEBUG = true;
#else
static constexpr bool SERENITY_DEBUG = false;
#endif

// STL includes.
#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <format>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <ranges>
#include <source_location>
#include <span>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeinfo>
#include <variant>
#include <vector>

using namespace std::string_literals;

// D3D12 / Windows includes.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <DirectXMath.h>
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

namespace math = DirectX;

// Global project includes.
#include "core/log.hpp"
#include "utils/primitive_datatypes.hpp"
