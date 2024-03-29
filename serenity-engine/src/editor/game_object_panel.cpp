#include "serenity-engine/editor/game_object_panel.hpp"

#include "imgui.h"

namespace serenity::editor
{
    void GameObjectPanel::render_panel_for_game_object(scene::Scene &scene, scene::GameObject &game_object)
    {
        ImGui::SetNextItemOpen(true);
        if (ImGui::Begin("Game Object Panel"))
        {
            ImGui::SetNextItemOpen(true);
            if (ImGui::TreeNode(game_object.game_object_name.c_str()))
            {
                ImGui::SetNextItemOpen(true);
                if (ImGui::TreeNode("Transform"))
                {
                    auto &transform_component = game_object.transform_component;

                    ImGui::SliderFloat3("S", &transform_component.scale.x, 0.1f, 10.0f);
                    ImGui::SliderFloat3("R", &transform_component.rotation.x, -180.0f, 180.0f);
                    ImGui::SliderFloat3("T", &transform_component.translation.x, -100.0f, 100.0f);

                    ImGui::TreePop();
                }

                auto &selected_script_index = game_object.script_index;
                if (const auto &scripts = scripting::ScriptManager::instance().get_scripts();
                    ImGui::BeginCombo("Selected Script", selected_script_index == INVALID_INDEX_U32
                                                             ? "None"
                                                             : scripts.at(selected_script_index).script_name.c_str()))
                {
                    for (const auto i : std::views::iota(0u, static_cast<uint32_t>(scripts.size() + 1u)))
                    {
                        const bool is_selected = (selected_script_index == i);

                        if (i == scripts.size())
                        {
                            if (ImGui::Selectable("None", is_selected))
                            {
                                selected_script_index = INVALID_INDEX_U32;
                            }
                        }
                        else if (ImGui::Selectable(scripts[i].script_name.c_str(), is_selected))
                        {
                            selected_script_index = i;
                        }

                        if (is_selected)
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                    }

                    ImGui::EndCombo();
                }

                if (ImGui::TreeNode("Material"))
                {
                    for (int i = 0; i < game_object.material_count; i++)
                    {
                        if (ImGui::TreeNode(std::string("Material "s + std::to_string(i)).c_str()))
                        {
                            auto &material_buffer_data =
                                scene.get_scene_resources().material_buffers.at(game_object.material_buffer_offset + i);

                            ImGui::ColorPicker3("Base Color", &material_buffer_data.base_color.x);
                            ImGui::SliderFloat("Metallic Factor", &material_buffer_data.metallic_roughness_factor.x,
                                               0.0f, 1.0f);
                            ImGui::SliderFloat("Roughness Factor", &material_buffer_data.metallic_roughness_factor.y,
                                               0.0f, 1.0f);

                            ImGui::TreePop();
                        }
                    }

                    ImGui::TreePop();
                }

                ImGui::TreePop();
            }

            ImGui::End();
        }
    }
} // namespace serenity::editor