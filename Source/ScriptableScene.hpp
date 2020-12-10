#ifndef SCRIPTABLE_SCENE_HPP
#define SCRIPTABLE_SCENE_HPP

#include "Engine/Scene.h"
#include "ScriptHooks.hpp"

LUA_SCRIPT_HOOK_DECLARATION(Scene, addMesh)

LUA_SCRIPT_HOOK_DECLARATION(Scene, addMeshInstance)

LUA_SCRIPT_HOOK_DECLARATION(Scene, removeMeshInstance)

#endif
