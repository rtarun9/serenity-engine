#include "serenity-engine/scripting/script_manager.hpp"

namespace serenity::scripting
{
    ScriptManager::ScriptManager()
    {
        m_lua.open_libraries(sol::lib::base);
        m_lua.open_libraries(sol::lib::math);

        // Set simple log function in lua scripts.
        m_lua.set_function("log_info", [&](const std::string message) { core::Log::instance().info(message); });
        m_lua.set_function("log_warn", [&](const std::string message) { core::Log::instance().warn(message); });
        m_lua.set_function("log_error", [&](const std::string message) { core::Log::instance().error(message); });

        // Set usertypes.

        // DirectXMath's float3.
        m_lua.new_usertype<math::XMFLOAT3>("float3", "x", &math::XMFLOAT3::x, "y", &math::XMFLOAT3::y, "z",
                                           &math::XMFLOAT3::z);
    }

    uint32_t ScriptManager::create_script(const Script &script)
    {
        const auto script_index = m_scripts.size();

        m_scripts.emplace_back(script);

        core::Log::instance().info(std::format("Created script with path : {}", script.script_path));

        return script_index;
    }

    void ScriptManager::execute_script(const uint32_t script_index)
    {
        m_lua.script_file(m_scripts.at(script_index).script_path, [&](lua_State *, sol::protected_function_result pfr) {
            if (const auto script_path = m_scripts.at(script_index).script_path; !pfr.valid())
            {
                core::Log::instance().warn(std::format("Error in script : {}", script_path));
            }

            return pfr;
        });
    }
} // namespace serenity::scripting