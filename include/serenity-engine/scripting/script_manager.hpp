#pragma once

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

namespace serenity::scripting
{
    // Light weight abstraction for script management + sol2.
    class ScriptManager : public core::SingletonInstance<ScriptManager>
    {
      public:
        explicit ScriptManager();

        sol::state& call_function()
        {
            return m_lua;
        }

        void execute_script(const std::string_view script_path);

      private:
        sol::state m_lua{};
    };
} // namespace serenity::scripting