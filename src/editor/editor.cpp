#include "serenity-engine/editor/editor.hpp"

#include "serenity-engine/core/file_system.hpp"
#include "serenity-engine/graphics/device.hpp"

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
        (void)io;

        m_ini_path = core::FileSystem::instance().get_relative_path("data/editor.ini");
        io.IniFilename = m_ini_path.c_str();

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable docking.

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        const auto current_srv_descriptor =
            graphics::Device::instance().get_cbv_srv_uav_descriptor_heap().get_current_handle();

        ImGui_ImplSDL3_InitForD3D(window.get_internal_window());
        ImGui_ImplDX12_Init(graphics::Device::instance().get_device().Get(), graphics::Swapchain::NUM_BACK_BUFFERS,
                            DXGI_FORMAT_R8G8B8A8_UNORM,
                            graphics::Device::instance().get_cbv_srv_uav_descriptor_heap().get_descriptor_heap().Get(),
                            current_srv_descriptor.cpu_descriptor_handle, current_srv_descriptor.gpu_descriptor_handle);

        graphics::Device::instance().get_cbv_srv_uav_descriptor_heap().offset_current_handle();

        window.add_event_callback([&](SDL_Event event) { ImGui_ImplSDL3_ProcessEvent(&event); });

        core::Log::instance().info("Created the editor");
    }

    Editor::~Editor()
    {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();

        core::Log::instance().info("Destroyed the editor");
    }

    void Editor::render(graphics::CommandList &command_list)
    {
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(),
                                     ImGuiDockNodeFlags_::ImGuiDockNodeFlags_PassthruCentralNode);

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Controls"))
            {
                ImGui::BulletText("Use WASD to move camera, and arror to change camera orientation");
                ImGui::BulletText("Left click and drag editor components to move them");
                ImGui::BulletText("Left click + Space to move the actual window");

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        ImGui::ShowDemoWindow();

        if (ImGui::Begin("Test"))
        {
            ImGui::End();
        }

        ImGui::Render();

        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), command_list.get_command_list().Get());
    }
} // namespace serenity::editor