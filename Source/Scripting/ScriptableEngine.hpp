#ifndef SCRIPTABLE_ENGINE_HPP
#define SCRIPTABLE_ENGINE_HPP

#include "../TempestEngine.hpp"
#include "ScriptHooks.hpp"

namespace Tempest {
    class ScriptEngine;


    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, getInstancePosition)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, setInstancePosition)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, translateInstance)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, startAnimation)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, terimateAnimation)


    void registerEngineLuaHooks(ScriptEngine *eng, TempestEngine *scene);

}

#endif