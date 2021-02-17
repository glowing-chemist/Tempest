#ifndef SCRIPT_ENGINE_HPP
#define SCRIPT_ENGINE_HPP

#include "ScriptHooks.hpp"

#include <chrono>
#include <type_traits>
#include <string>
#include <vector>
#include <unordered_map>


class RenderEngine;
class Scene;

namespace Tempest
{

enum ScriptContext : uint32_t
{
    kContext_GamePlay = 0,
    kContext_Physics,
    kContext_Graphics,

    kContext_Count

};

template<typename T>
struct ExtractClassType
{
    using CLASS = void;
    using RETURN = void;
};

template<typename C, typename R, typename ...Args>
struct ExtractClassType<R (C::*)(Args...)>
{
    using CLASS = C;
    using RETURN = R;
};

template<typename C, typename R, typename ...Args>
struct ExtractClassType<R (C::*)(Args...) const>
{
    using CLASS = C;
    using RETURN = R;
};

class ScriptableCallableBase
{
public:
    ScriptableCallableBase() = default;
    ~ScriptableCallableBase() = default;

    virtual int callFunction(lua_State* L) = 0;
};

extern std::unordered_map<std::string, int(*)(lua_State*)> s_dispatchFunctions;

template<typename F, typename ...Args>
class ScriptableCallable : public ScriptableCallableBase
{
public:

    ScriptableCallable(F f, typename ExtractClassType<F>::CLASS* s) :
        mSystem(s), mF(f) {}


    virtual int callFunction(lua_State* L) override
    {
        return executeCallback<F, typename ExtractClassType<F>::CLASS, Args...>(L, mF, mSystem);
    }

private:

    typename ExtractClassType<F>::CLASS* mSystem;
    F mF;
};


class CallablesRegistrar
{
public:
    CallablesRegistrar() {}
    ~CallablesRegistrar() = default;

    void registerLuaCallable(const std::string& name, ScriptableCallableBase* callable)
    {
        mCallables.insert({name, callable});
    }

    const std::unordered_map<std::string, ScriptableCallableBase*>& getCallables() const
    {
        return mCallables;
    }

private:

    std::unordered_map<std::string, ScriptableCallableBase*> mCallables;
};


class ScriptEngine
{
public:

    ScriptEngine();
    ~ScriptEngine();

    void tick(const std::chrono::microseconds);

    void registerScript(const std::string& path, const std::string& func, const ScriptContext);

    void registerEntityWithScript(const std::string& func, const int64_t entity, const ScriptContext);

    CallablesRegistrar* createCallablesRegistrar()
    {
        return new CallablesRegistrar{};
    }

    void registerCallables(CallablesRegistrar* registrar)
    {
        const std::unordered_map<std::string, ScriptableCallableBase*>& callables = registrar->getCallables();
        for(const auto& [name, callable] : callables)
        {
            int(*f)(lua_State*) = s_dispatchFunctions[name];
            lua_register(mState, name.c_str(), f);

            mCallables.insert({name, callable});
        }

        delete registrar;
    }

    ScriptableCallableBase* getCallableByName(const std::string& name)
    {
        BELL_ASSERT(mCallables.find(name) != mCallables.end(), "Can't find script callcable")
        return mCallables[name];
    }

    void registerSceneHooks(Scene*);

private:

    void pushArgsOnStack(const int64_t entity, const ScriptContext);

    void call_lua_func(const char *f, const uint32_t args, const uint32_t returns);

    void load_script(const char* f);

    std::unordered_map<std::string, std::vector<int64_t>> mComponentScripts[kContext_Count];

    std::unordered_map<std::string, ScriptableCallableBase*> mCallables;

    lua_State* mState;
};

ScriptEngine* getScriptEngine();

}

#endif
