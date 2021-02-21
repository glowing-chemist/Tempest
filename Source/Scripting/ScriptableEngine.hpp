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

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, getInstanceIDByName)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, getSceneIDByName)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, setMainCameraByName)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, setShadowCameraByName)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, createPlayerInstance)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, updatePlayerInstance)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, attachCameraToPlayer)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, attachShadowCameraToPlayer)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, updateControllerInstance)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, attachCameraToPlayer)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, attachShadowCameraToPlayer)

    void registerEngineLuaHooks(ScriptEngine *eng, TempestEngine *scene);

}

#endif