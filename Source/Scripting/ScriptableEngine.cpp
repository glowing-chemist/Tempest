#include "ScriptableEngine.hpp"
#include "ScriptEngine.hpp"
#include "Controller.hpp"

namespace Tempest
{
    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, getInstancePosition)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, setInstancePosition)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, setInstanceRotation)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, translateInstance)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, startAnimation)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, terminateAnimation)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, getInstanceIDByName)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, getSceneIDByName)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, setMainCameraByName)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, setShadowCameraByName)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, createPlayerInstance)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, getControllerForInstance)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, attachCameraToPlayer)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, attachShadowCameraToPlayer)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, createControllerInstance)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, updateControllerInstance)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, getPhysicsBodyPosition)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, applyImpulseToInstance)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, setGraphicsInstancePosition)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, updatePlayersAttachedCameras)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, getCameraDirectionByName)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, getCameraRightByName)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, getCameraPositionByName)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, getInstanceSize)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, getInstanceCenter)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, startInstanceFrame)

    void registerEngineLuaHooks(ScriptEngine *scriptEngine, TempestEngine *engine)
    {
        CallablesRegistrar *registrar = scriptEngine->createCallablesRegistrar();

        LUA_REGISTER_HOOK(TempestEngine, getInstancePosition, engine, InstanceID)

        LUA_REGISTER_HOOK(TempestEngine, setInstancePosition, engine, InstanceID, float3)

        LUA_REGISTER_HOOK(TempestEngine, setInstanceRotation, engine, InstanceID, quat)

        LUA_REGISTER_HOOK(TempestEngine, translateInstance, engine, InstanceID, float3)

        LUA_REGISTER_HOOK(TempestEngine, startAnimation, engine, InstanceID, std::string, bool, float)

        LUA_REGISTER_HOOK(TempestEngine, terminateAnimation, engine, InstanceID, std::string)

        LUA_REGISTER_HOOK(TempestEngine, getInstanceIDByName, engine, std::string)

        LUA_REGISTER_HOOK(TempestEngine, getSceneIDByName, engine, std::string)

        LUA_REGISTER_HOOK(TempestEngine, setMainCameraByName, engine, std::string)

        LUA_REGISTER_HOOK(TempestEngine, setShadowCameraByName, engine, std::string)

        LUA_REGISTER_HOOK(TempestEngine, createPlayerInstance, engine, InstanceID, float3, float3)

        LUA_REGISTER_HOOK(TempestEngine, getControllerForInstance, engine, InstanceID)

        LUA_REGISTER_HOOK(TempestEngine, attachCameraToPlayer, engine, InstanceID, std::string, float)

        LUA_REGISTER_HOOK(TempestEngine, attachShadowCameraToPlayer, engine, InstanceID, std::string)

        LUA_REGISTER_HOOK(TempestEngine, createControllerInstance, engine, InstanceID, uint32_t)

        LUA_REGISTER_HOOK(TempestEngine, updateControllerInstance, engine, InstanceID)

        LUA_REGISTER_HOOK(TempestEngine, getPhysicsBodyPosition, engine, InstanceID)

        LUA_REGISTER_HOOK(TempestEngine, applyImpulseToInstance, engine, InstanceID, float3)

        LUA_REGISTER_HOOK(TempestEngine, setGraphicsInstancePosition, engine, InstanceID, float3)

        LUA_REGISTER_HOOK(TempestEngine, updatePlayersAttachedCameras, engine, InstanceID)

        LUA_REGISTER_HOOK(TempestEngine, getCameraDirectionByName, engine, std::string)

        LUA_REGISTER_HOOK(TempestEngine, getCameraPositionByName, engine, std::string)

        LUA_REGISTER_HOOK(TempestEngine, getCameraRightByName, engine, std::string)

        LUA_REGISTER_HOOK(TempestEngine, getInstanceSize, engine, InstanceID)

        LUA_REGISTER_HOOK(TempestEngine, getInstanceCenter, engine, InstanceID)

        LUA_REGISTER_HOOK(TempestEngine, startInstanceFrame, engine, InstanceID)

        scriptEngine->registerCallables(registrar);
    }

    void pushLuaStack(lua_State *L, const Controller& c)
    {
        lua_createtable(L, 0, 7);
        setLuaTableEntry(L, "Lx", c.getLeftAxisX());
        setLuaTableEntry(L, "Ly", c.getLeftAxisY());
        setLuaTableEntry(L, "Rx", c.getRightAxisX());
        setLuaTableEntry(L, "Ry", c.getRightAxisY());
        setLuaTableEntry(L, "LCtl", c.ctrlPressed());
        setLuaTableEntry(L, "LShft", c.shftPressed());
        setLuaTableEntry(L, "X", c.pressedX());
    }
}