#include "ScriptableScene.hpp"
#include "ScriptEngine.hpp"

LUA_SCRIPT_HOOK_DEFINITION(Scene, addMesh)

LUA_SCRIPT_HOOK_DEFINITION(Scene, addMeshInstance)

LUA_SCRIPT_HOOK_DEFINITION(Scene, removeMeshInstance)

LUA_SCRIPT_HOOK_DEFINITION(Scene, testPrint)

void registerSceneLuaHooks(Tempest::ScriptEngine* eng, Scene* scene)
{
    Tempest::CallablesRegistrar* registrar = eng->createCallablesRegistrar();

    LUA_REGISTER_HOOK(Scene, testPrint, scene)

    eng->registerCallables(registrar);
}
