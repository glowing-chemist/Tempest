#ifndef SCRIPT_ENGINE_HPP
#define SCRIPT_ENGINE_HPP

#include "lua.hpp"

#include <chrono>

class Engine;
class Scene;

class ScriptEngine
{
public:

    ScriptEngine(Engine* eng, Scene*);
    ~ScriptEngine();

    void tick(std::chrono::microseconds);

private:

    void call_lua_func(const char *f, const uint32_t args, const uint32_t returns);

    void load_script(const char* f);

    lua_State* mState;
    Engine* mEngine;
    Scene* mScene;
};

#endif
