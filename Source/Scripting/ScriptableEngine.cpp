#include "ScriptableEngine.hpp"
#include "ScriptEngine.hpp"

namespace Tempest
{
    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, getInstancePosition)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, setInstancePosition)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, translateInstance)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, startAnimation)

    LUA_SCRIPT_HOOK_DEFINITION(TempestEngine, terimateAnimation)

    void registerEngineLuaHooks(ScriptEngine *scriptEngine, TempestEngine *engine)
    {
        CallablesRegistrar *registrar = scriptEngine->createCallablesRegistrar();

        LUA_REGISTER_HOOK(TempestEngine, getInstancePosition, engine, InstanceID)

        LUA_REGISTER_HOOK(TempestEngine, setInstancePosition, engine, InstanceID, float3)

        LUA_REGISTER_HOOK(TempestEngine, translateInstance, engine, InstanceID, float3)

        LUA_REGISTER_HOOK(TempestEngine, startAnimation, engine, InstanceID, std::string, uint32_t, float)

        LUA_REGISTER_HOOK(TempestEngine, terimateAnimation, engine, InstanceID, std::string)

        scriptEngine->registerCallables(registrar);
    }

}