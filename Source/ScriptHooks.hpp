#ifndef SCRIPT_HOOKS_HPP
#define SCRIPT_HOOKS_HPP

#include <cstdint>
#include <functional>
#include <string>
#include <type_traits>
#include <utility>

#include "lua.hpp"
#include "Core/BellLogging.hpp"

#define LUA_SCRIPT_HOOK_NAME(C, F) #C "_" #F

#define LUA_SCRIPT_HOOK_DECLARATION(C, F) int C ## _ ## F(lua_State*);

#define LUA_SCRIPT_HOOK_DEFINITION(C, F) int C ## _ ## F(lua_State* L)  \
{									\
    Tempest::ScriptEngine* se = Tempest::getScriptEngine();				\
    Tempest::ScriptableCallableBase* callable = se->getCallableByName(LUA_SCRIPT_HOOK_NAME(C, F)); \
    return callable->callFunction(L);					\
}

#define LUA_REGISTER_HOOK(C, F, I) \
    { \
        auto* callable = new Tempest::ScriptableCallable(&C::F, I); \
        const std::string name = LUA_SCRIPT_HOOK_NAME(C, F); \
        registrar->registerLuaCallable(name, callable); \
        Tempest::s_dispatchFunctions[name] = &C ## _ ## F;      \
    }

template<typename ...S>
struct LuaStack{};

template<typename T>
T popLuaStack(lua_State*, const uint32_t)
{
    BELL_TRAP;
}


template<>
inline int popLuaStack(lua_State* L, const uint32_t i)
{
    return  lua_tointeger(L, i);
}

template<>
inline lua_Number popLuaStack(lua_State* L, const uint32_t i)
{
    return  lua_tonumber(L, i);
}

template<>
inline std::string popLuaStack(lua_State* L, const uint32_t i)
{
    return lua_tostring(L, i);
}


template<typename T>
void pushLuaStack(lua_State*, const T)
{
    BELL_TRAP;
}

template<>
inline void pushLuaStack(lua_State* L, const int i)
{
    lua_pushinteger(L, i);
}

template<>
inline void pushLuaStack(lua_State* L, const lua_Number n)
{
    lua_pushnumber(L, n);
}

template<>
inline void pushLuaStack(lua_State* L, const std::string s)
{
    lua_pushstring(L, s.c_str());
}

template<typename F, typename I, typename H, typename ...Stack, template <typename...> class S, typename...Args>
int executeCallback_impl(lua_State* L, F f, I* instance, const uint32_t stackDepth, S<H, Stack...>, Args ...args)
{
    const auto p = popLuaStack<H>(L, stackDepth);
    return executeCallback_impl(L, f, instance, stackDepth + 1, LuaStack<Stack...>{}, args..., p);
}

template<typename F, typename I, template <typename...> class S, typename...Args>
int executeCallback_impl(lua_State*, F f, I* instance, const uint32_t, S<>, Args ...args)
{
    if constexpr (std::is_same_v<typename std::invoke_result_t<F, I, Args...>, void>)
    {
        std::invoke(f, instance, args...);
        return 0;
    }
    else
    {
        const auto result = std::invoke(f, instance, args...);
        pushLuaStack(result);
        return 1;
    }
}


template<typename F, typename I, typename ...Stack>
int executeCallback(lua_State* L, F f, I* instance)
{
    return executeCallback_impl(L, f, instance, 1u, LuaStack<Stack...>{});
}

#endif
