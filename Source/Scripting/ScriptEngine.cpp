#include "ScriptEngine.hpp"
#include "ScriptableScene.hpp"
#include "ScriptableEngine.hpp"
#include "ScriptableRenderer.hpp"

#include "Include/Engine/Engine.hpp"
#include "Include/Engine/Scene.h"

namespace Tempest
{

ScriptEngine* s_scriptEngine;
std::unordered_map<std::string, int(*)(lua_State*)> s_dispatchFunctions;

ScriptEngine::ScriptEngine() :
    mState(nullptr)
{
    mState = luaL_newstate();
    luaL_openlibs(mState);

    // load main game script.
    load_script("./Scripts/main.lua");

    lua_getglobal(mState, "init");
    call_lua_func("init", 0, 0);

    s_scriptEngine = this;
}


ScriptEngine::~ScriptEngine()
{
    lua_close(mState);
}


void ScriptEngine::tick(const std::chrono::microseconds delta)
{
    lua_getglobal(mState, "main");

    lua_pushinteger(mState, delta.count());
    call_lua_func("main", 1, 0);

    for(uint32_t iContext = 0; iContext < kContext_Count; ++iContext)
    {
        for(const auto&[name, entities] : mComponentScripts[iContext])
        {
            for(const auto entity : entities)
            {
                lua_pushinteger(mState, entity);
                lua_pushinteger(mState, delta.count());
                call_lua_func(name.c_str(), 2, 0);
            }
        }
    }
}


void ScriptEngine::registerScript(const std::string& path, const std::string& func, const ScriptContext context)
{
    load_script(path.c_str());
    mComponentScripts[context].insert({func, {}});
}


void ScriptEngine::registerEntityWithScript(const std::string& func, const int64_t entity, const ScriptContext context)
{
    mComponentScripts[context][func].push_back(entity);
}



void ScriptEngine::call_lua_func(const char* f, const uint32_t args, const uint32_t returns)
{
    if (lua_pcall(mState, args, returns, 0) != 0)
    {
        BELL_LOG_ARGS("error running function %s: %s\n", f, lua_tostring(mState, -1));
        BELL_TRAP;
    }
}


void ScriptEngine::load_script(const char* f)
{
    const bool error = luaL_loadfile(mState, f) || lua_pcall(mState, 0, 0, 0);
    BELL_ASSERT(!error, "Failed to load script file")
}

void ScriptEngine::registerSceneHooks(Scene* s)
{
    registerSceneLuaHooks(this, s);
}


void ScriptEngine::registerEngineHooks(TempestEngine* engine)
{
    registerEngineLuaHooks(this, engine);
}


void ScriptEngine::registerPhysicsHooks(PhysicsWorld*)
{

}


ScriptEngine* getScriptEngine()
{
    return s_scriptEngine;
}

}
