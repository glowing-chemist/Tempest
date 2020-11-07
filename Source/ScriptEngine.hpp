#ifndef SCRIPT_ENGINE_HPP
#define SCRIPT_ENGINE_HPP

#include "lua.hpp"

#include <chrono>
#include <string>
#include <vector>
#include <unordered_map>


class Engine;
class Scene;

enum ScriptContext : uint32_t
{
    kContext_GamePlay = 0,
    kContext_Physics,
    kContext_Graphics,

    kContext_Count

};

class ScriptEngine
{
public:

    ScriptEngine(Engine* eng, Scene*);
    ~ScriptEngine();

    void tick(const std::chrono::microseconds);

    void registerScript(const std::string& path, const std::string& func, const ScriptContext);

    void registerEntityWithScript(const std::string& func, const int64_t entity, const ScriptContext);

private:

    void pushArgsOnStack(const int64_t entity, const ScriptContext);

    void call_lua_func(const char *f, const uint32_t args, const uint32_t returns);

    void load_script(const char* f);

    std::unordered_map<std::string, std::vector<int64_t>> mComponentScripts[kContext_Count];

    lua_State* mState;
    Engine* mEngine;
    Scene* mScene;
};

#endif
