#include "ScriptEngine.hpp"

#include "Include/Engine/Engine.hpp"
#include "Include/Engine/Scene.h"


ScriptEngine::ScriptEngine(Engine* eng, Scene* scene) :
    mState(nullptr),
    mEngine(eng),
    mScene(scene)
{
    mState = luaL_newstate();
    luaL_openlibs(mState);

    // load main game script.
    load_script("./Scripts/main.lua");

    lua_getglobal(mState, "init");
    call_lua_func("init", 0, 0);
}


ScriptEngine::~ScriptEngine()
{
    lua_close(mState);
}


void ScriptEngine::tick(std::chrono::microseconds delta)
{
    lua_getglobal(mState, "main");

    lua_pushinteger(mState, delta.count());
    call_lua_func("main", 1, 0);
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