#ifndef SCRIPTABLE_ENGINE_HPP
#define SCRIPTABLE_ENGINE_HPP

#include "../TempestEngine.hpp"
#include "ScriptHooks.hpp"

namespace Tempest {
    class ScriptEngine;


    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, getInstancePosition)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, setInstancePosition)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, setInstanceRotation)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, translateInstance)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, startAnimation)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, terminateAnimation)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, getInstanceIDByName)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, getSceneIDByName)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, setMainCameraByName)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, setShadowCameraByName)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, createPlayerInstance)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, getControllerForInstance)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, attachCameraToPlayer)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, attachShadowCameraToPlayer)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, updateControllerInstance)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, attachCameraToPlayer)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, attachShadowCameraToPlayer)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, getPhysicsBodyPosition)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, applyImpulseToInstance)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, setGraphicsInstancePosition)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, updatePlayersAttachedCameras)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, getCameraDirectionByName)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, getCameraRightByName)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, getCameraPositionByName)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, getInstanceSize)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, getInstanceCenter)

    LUA_SCRIPT_HOOK_DECLARATION(TempestEngine, startInstanceFrame)

    void registerEngineLuaHooks(ScriptEngine *eng, TempestEngine *scene);

    void pushLuaStack(lua_State *L, const Controller&);
}

#endif