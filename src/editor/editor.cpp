#include "serenity-engine/editor/editor.hpp"

#include "serenity-engine/core/file_system.hpp"
#include "serenity-engine/renderer/renderer.hpp"
#include "serenity-engine/scene/scene_manager.hpp"

#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_sdl3.h"

#include "SDL_events.h"

namespace serenity::editor
{
    Editor::Editor(window::Window &window)
    {
        // Setup code from here :
        // https://github.com/ocornut/imgui/blob/master/examples/example_win32_directx12/main.cpp.

        // Setup Dear ImGui context.
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();

        m_ini_path = core::FileSystem::instance().get_absolute_path("data/editor.ini");
        io.IniFilename = m_ini_path.c_str();

        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable docking.

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        ImVec4 *colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_FrameBg] = ImVec4(0.48f, 0.16f, 0.16f, 0.54f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.98f, 0.26f, 0.26f, 0.67f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.48f, 0.16f, 0.16f, 1.00f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.98f, 0.26f, 0.26f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.88f, 0.24f, 0.24f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.98f, 0.26f, 0.26f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(0.98f, 0.26f, 0.26f, 0.40f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.98f, 0.26f, 0.26f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.98f, 0.06f, 0.06f, 1.00f);
        colors[ImGuiCol_Header] = ImVec4(0.98f, 0.26f, 0.26f, 0.31f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.98f, 0.26f, 0.26f, 0.80f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.98f, 0.26f, 0.26f, 1.00f);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.75f, 0.10f, 0.10f, 0.78f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.75f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.98f, 0.26f, 0.26f, 0.20f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.98f, 0.26f, 0.26f, 0.67f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.98f, 0.26f, 0.26f, 0.95f);
        colors[ImGuiCol_Tab] = ImVec4(0.58f, 0.18f, 0.18f, 0.86f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.98f, 0.26f, 0.26f, 0.80f);
        colors[ImGuiCol_TabActive] = ImVec4(0.68f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.42f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_DockingPreview] = ImVec4(0.98f, 0.26f, 0.26f, 0.70f);

        const auto current_srv_descriptor =
            renderer::Renderer::instance().get_device().get_cbv_srv_uav_descriptor_heap().get_current_handle();

        ImGui_ImplSDL3_InitForD3D(window.get_internal_window());
        ImGui_ImplDX12_Init(
            renderer::Renderer::instance().get_device().get_device().Get(), renderer::rhi::Swapchain::NUM_BACK_BUFFERS,
            renderer::rhi::Swapchain::SWAPCHAIN_BACK_BUFFER_FORMAT,
            renderer::Renderer::instance().get_device().get_cbv_srv_uav_descriptor_heap().get_descriptor_heap().Get(),
            current_srv_descriptor.cpu_descriptor_handle, current_srv_descriptor.gpu_descriptor_handle);

        renderer::Renderer::instance().get_device().get_cbv_srv_uav_descriptor_heap().offset_current_handle();

        window.add_event_callback([&](window::Event event) { ImGui_ImplSDL3_ProcessEvent(&event.internal_event); });

        core::Log::instance().info("Created the editor");
    }

    Editor::~Editor()
    {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();

        core::Log::instance().info("Destroyed the editor");
    }

    void Editor::render()
    {
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(),
                                     ImGuiDockNodeFlags_::ImGuiDockNodeFlags_PassthruCentralNode);

        static auto render_ui = true;

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Controls"))
            {
                ImGui::BulletText("Use WASD to move camera, and arror to change camera orientation");
                ImGui::BulletText("Left click and drag editor components to move them");
                ImGui::BulletText("Left click + Space to move the actual window");

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Editor Settings"))
            {
                ImGui::Checkbox("Render Editor UI", &render_ui);
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        if (render_ui)
        {
            scene_panel();
            renderer_panel();
        }

        ImGui::Render();

        auto &command_list =
            renderer::Renderer::instance().get_device().get_current_frame_direct_command_list().get_command_list();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), command_list.Get());
    }

    void Editor::scene_panel()
    {
        auto &current_scene = scene::SceneManager::instance().get_current_scene();

        ImGui::SetNextItemOpen(true);
        if (ImGui::Begin(current_scene.get_scene_name().c_str()))
        {
            ImGui::SetNextItemOpen(true);
            if (ImGui::TreeNode("Scene Hierarchy"))
            {
                for (auto &game_object : current_scene.get_game_objects())
                {
                    if (ImGui::TreeNode(game_object.m_game_object_name.c_str()))
                    {
                        if (ImGui::TreeNode("Transform"))
                        {
                            auto &transform_component = game_object.m_transform_component;

                            ImGui::SliderFloat("S", &transform_component.scale.x, 0.1f, 10.0f);
                            ImGui::SliderFloat3("R", &transform_component.rotation.x, -180.0f, 180.0f);
                            ImGui::SliderFloat3("T", &transform_component.translation.x, -100.0f, 100.0f);

                            transform_component.scale.y = transform_component.scale.x;
                            transform_component.scale.z = transform_component.scale.x;

                            ImGui::TreePop();
                        }

                        ImGui::TreePop();
                    }
                }

                ImGui::TreePop();
            }

            ImGui::SetNextItemOpen(true);
            if (auto &camera = current_scene.get_camera(); ImGui::TreeNode("Camera Settings"))
            {
                ImGui::SliderFloat("Movement speed", &camera.m_movement_speed, 0.0001f, 1.0f);
                ImGui::SliderFloat("Rotation speed", &camera.m_rotation_speed, 0.0001f, 0.10f);
                ImGui::SliderFloat("Friction", &camera.m_friction_factor, 0.0001f, 1.0f);

                ImGui::TreePop();
            }

            ImGui::SetNextItemOpen(true);
            if (auto &light_buffer = current_scene.get_lights().get_light_buffer(); ImGui::TreeNode("Light Settings"))
            {
                if (ImGui::TreeNode("Directional Light"))
                {
                    ImGui::SliderFloat("Sun angle", &light_buffer.sun_angle, -180.0f,
                                       0.0f);
                    ImGui::SliderFloat("Intensity", &light_buffer.lights[interop::SUN_LIGHT_INDEX].intensity, 0.0f,
                                       1.0f);
                    ImGui::TreePop();
                }

                for (size_t i = 1; i < light_buffer.light_count; i++)
                {
                    if (ImGui::TreeNode(std::string("Light " + std::to_string(i)).c_str()))
                    {
                        ImGui::SliderFloat3("Position", &light_buffer.lights[i].world_space_position_or_direction.x,
                                            -50.0f, 50.0f);
                        ImGui::ColorEdit3("Color", &light_buffer.lights[i].color.x);
                        ImGui::SliderFloat("Intensity", &light_buffer.lights[i].intensity, 0.01f, 5.0f);
                        ImGui::SliderFloat("Scale", &light_buffer.lights[i].scale, 0.01f, 10.0f);

                        ImGui::TreePop();
                    }
                }

                ImGui::TreePop();
            }

            ImGui::End();
        }
    }

    void Editor::renderer_panel()
    {
        ImGui::SetNextItemOpen(true);
        if (ImGui::Begin("Renderer Panel"))
        {
            auto &atmosphere_buffer = renderer::Renderer::instance().get_atmosphere_renderpass_buffer();

            if (ImGui::TreeNode("Atmosphere Renderpass"))
            {
                ImGui::SliderFloat("Turbidity", &atmosphere_buffer.turbidity, 2.0f, 10.0f);

                ImGui::TreePop();
            }

            ImGui::SetNextItemOpen(true);
            if (const auto &pipelines = renderer::Renderer::instance().get_pipelines(); ImGui::TreeNode("Pipelines"))
            {
                for (const auto &pipeline : pipelines)
                {
                    if (const auto pipeline_name = pipeline.pipeline_creation_desc.name;
                        ImGui::Button(std::string("Reload "s + wstring_to_string(pipeline_name)).c_str()))
                    {
                        renderer::Renderer::instance().schedule_pipeline_for_reload(pipeline.index);
                    }
                }
                ImGui::TreePop();
            }

            ImGui::End();
        }
    }
} // namespace serenity::editor