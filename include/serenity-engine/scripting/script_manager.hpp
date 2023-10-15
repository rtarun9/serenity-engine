#pragma once

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

namespace serenity::scripting
{
    struct Script
    {
        std::string script_name{};
        std::string script_path{};
    };

    // Light weight abstraction for script management + sol2.
    // Has a vector of scripts, and the various engine/game scriptable-things can use a index to access the script.
    class ScriptManager : public core::SingletonInstance<ScriptManager>
    {
      public:
        explicit ScriptManager();

        std::vector<Script> &get_scripts()
        {
            return m_scripts;
        }

        sol::state &call_function()
        {
            return m_lua;
        }

        // Returns a script_index, which can be used to index into the scripts vector and access the script.
        uint32_t create_script(const Script& script);

        void execute_script(const uint32_t script_index);

      private:
        sol::state m_lua{};

        std::vector<Script> m_scripts{};
    };
} // namespace serenity::scripting