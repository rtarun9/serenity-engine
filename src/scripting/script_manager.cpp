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
        m_lua.new_usertype<math::XMFLOAT3>("float3", "x", &math::XMFLOAT3::x, "y", &math::XMFLOAT3::y, "z",
                                           &math::XMFLOAT3::z);
    }

    void ScriptManager::execute_script(const std::string_view script_path)
    {
        m_lua.script_file(std::string(script_path), [&](lua_State *, sol::protected_function_result pfr) {
            if (!pfr.valid())
            {
                core::Log::instance().warn(std::format("Error in script : {}", script_path));
            }

            return pfr;
        });
    }
} // namespace serenity::scripting