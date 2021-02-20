#include "ScriptableEngine.hpp"
#include "ScriptEngine.hpp"

namespace Tempest
{
    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, getInstancePosition)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, setInstancePosition)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, translateInstance)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, startAnimation)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, terimateAnimation)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, getInstanceIDByName)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, getSceneIDByName)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, setMainCameraByName)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, setShadowCameraByName)

    void registerEngineLuaHooks(ScriptEngine *scriptEngine, TempestEngine *engine)
    {
        CallablesRegistrar *registrar = scriptEngine->createCallablesRegistrar();

        LUA_REGISTER_HOOK(TempestEngine, getInstancePosition, engine, InstanceID)

        LUA_REGISTER_HOOK(TempestEngine, setInstancePosition, engine, InstanceID, float3)

        LUA_REGISTER_HOOK(TempestEngine, translateInstance, engine, InstanceID, float3)

        LUA_REGISTER_HOOK(TempestEngine, startAnimation, engine, InstanceID, std::string, uint32_t, float)

        LUA_REGISTER_HOOK(TempestEngine, terimateAnimation, engine, InstanceID, std::string)

        LUA_REGISTER_HOOK(TempestEngine, getInstanceIDByName, engine, std::string)

        LUA_REGISTER_HOOK(TempestEngine, getSceneIDByName, engine, std::string)

        LUA_REGISTER_HOOK(TempestEngine, setMainCameraByName, engine, std::string)

        LUA_REGISTER_HOOK(TempestEngine, setShadowCameraByName, engine, std::string)

        scriptEngine->registerCallables(registrar);
    }

}