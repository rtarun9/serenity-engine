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
            atmosphere_panel();
        }

        ImGui::Render();

        auto &command_list =
            renderer::Renderer::instance().get_device().get_current_frame_direct_command_list().get_command_list();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), command_list.Get());
    }

    void Editor::scene_panel()
    {
        auto &current_scene = scene::SceneManager::instance().get_current_scene();

        if (auto &camrera = current_scene.get_camera(); ImGui::Begin("Camera Settings"))
        {
            ImGui::SliderFloat("Movement speed", &camrera.m_movement_speed, 0.0001f, 1.0f);
            ImGui::SliderFloat("Rotation speed", &camrera.m_rotation_speed, 0.0001f, 0.10f);
            ImGui::SliderFloat("Friction", &camrera.m_friction_factor, 0.0001f, 1.0f);

            ImGui::End();
        }

        if (ImGui::Begin("Sun Angle"))
        {
            ImGui::SliderFloat("Sun angle", &current_scene.get_scene_buffer().sun_angle, -180.0f, 0.0f);
         
            ImGui::End();
        }

        if (ImGui::Begin(current_scene.get_scene_name().c_str()))
        {
            for (auto &model : current_scene.get_models())
            {
                if (ImGui::TreeNode(model.model_name.c_str()))
                {
                    if (ImGui::TreeNode("Transform"))
                    {
                        auto &transform_component = model.transform_component;

                        ImGui::SliderFloat("S", &transform_component.scale.x, 0.1f, 10.0f);
                        ImGui::SliderFloat3("R", &transform_component.rotation.x, -180.0f, 180.0f);
                        ImGui::SliderFloat3("T", &transform_component.translation.x, -100.0f, 100.0f);

                        model.transform_component.scale.y = model.transform_component.scale.x;
                        model.transform_component.scale.z = model.transform_component.scale.x;

                        ImGui::TreePop();
                    }

                    ImGui::TreePop();
                }
            }

            ImGui::End();
        }
    }

    void Editor::atmosphere_panel()
    {
        auto &atmosphere_buffer = renderer::Renderer::instance().get_atmosphere_renderpass_buffer();

        ImGui::Begin("Atmosphere Settings");

        ImGui::SliderFloat("Turbidity", &atmosphere_buffer.turbidity, 2.0f, 10.0f);
        ImGui::SliderFloat("Magnitude Multiplier", &atmosphere_buffer.magnitude_multiplier, 0.0f, 1.0f);

        ImGui::End();
    }
} // namespace serenity::editor