#ifndef SCRIPTABLE_SCENE_HPP
#define SCRIPTABLE_SCENE_HPP

#include "Engine/Scene.h"
#include "ScriptHooks.hpp"

namespace Tempest
{
    class ScriptEngine;
}

LUA_SCRIPT_HOOK_DECLARATION(Scene, addMesh)

LUA_SCRIPT_HOOK_DECLARATION(Scene, addMeshInstance)

LUA_SCRIPT_HOOK_DECLARATION(Scene, removeMeshInstance)

LUA_SCRIPT_HOOK_DECLARATION(Scene, testPrint)

void registerSceneLuaHooks(Tempest::ScriptEngine* eng, Scene* scene);

#endif
