#include "ScriptableScene.hpp"
#include "ScriptEngine.hpp"

LUA_SCRIPT_HOOK_DEFINITION(Scene, addMesh)

LUA_SCRIPT_HOOK_DEFINITION(Scene, addMeshInstance)

LUA_SCRIPT_HOOK_DEFINITION(Scene, removeMeshInstance)

LUA_SCRIPT_HOOK_DEFINITION(Scene, getInstancePosition)

LUA_SCRIPT_HOOK_DEFINITION(Scene, setInstancePosition)

LUA_SCRIPT_HOOK_DEFINITION(Scene, testPrint)

void registerSceneLuaHooks(Tempest::ScriptEngine* eng, Scene* scene)
{
    Tempest::CallablesRegistrar* registrar = eng->createCallablesRegistrar();

    LUA_REGISTER_HOOK(Scene, testPrint, scene)

    LUA_REGISTER_HOOK(Scene, getInstancePosition, scene, InstanceID)

    LUA_REGISTER_HOOK(Scene, setInstancePosition, scene, InstanceID, float3)

    eng->registerCallables(registrar);
}
