#include "ScriptableScene.hpp"
#include "ScriptEngine.hpp"

namespace Tempest {

    LUA_SCRIPT_HOOK_DEFINITION(Scene, addMesh)

    LUA_SCRIPT_HOOK_DEFINITION(Scene, addMeshInstance)

    LUA_SCRIPT_HOOK_DEFINITION(Scene, removeMeshInstance)

    LUA_SCRIPT_HOOK_DEFINITION(Scene, testPrint)

    void registerSceneLuaHooks(Tempest::ScriptEngine *scriptEngine, Scene *scene)
    {
        Tempest::CallablesRegistrar *registrar = scriptEngine->createCallablesRegistrar();

        scriptEngine->registerCallables(registrar);
    }

}
