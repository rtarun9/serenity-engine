#pragma once

// Includes all header file in the project.
// Prevents the need to manually include selected engine header files in the game / applications.

// Asset
#include "asset/model_loader.hpp"
#include "asset/texture_loader.hpp"

// Core
#include "core/application.hpp"
#include "core/file_system.hpp"
#include "core/input.hpp"
#include "core/log.hpp"
#include "core/singleton_instance.hpp"

// Editor
#include "editor/editor.hpp"
#include "editor/imgui_sink.hpp"
#include "editor/game_object_panel.hpp"

// Renderer
#include "renderer/renderer.hpp"
#include "renderer/shader_compiler.hpp"
#include "renderer/shader.hpp"

// Renderer RHI
#include "renderer/rhi/rhi.hpp"

// Renderer renderpass
#include "renderer/renderpass/atmosphere_renderpass.hpp"
#include "renderer/renderpass/cube_map_renderpass.hpp"
#include "renderer/renderpass/shading_renderpass.hpp"
#include "renderer/renderpass/post_processing_renderpass.hpp"

// Scene
#include "scene/camera.hpp"
#include "scene/game_object.hpp"
#include "scene/lights.hpp"
#include "scene/scene.hpp"
#include "scene/scene_manager.hpp"

// Utils
#include "utils/enum_value.hpp"
#include "utils/primitive_datatypes.hpp"
#include "utils/string_conversions.hpp"
#include "utils/timer.hpp"

// Window
#include "window/window.hpp"

// Scripting
#include "scripting/script_manager.hpp"