#ifndef SCRIPT_HOOKS_HPP
#define SCRIPT_HOOKS_HPP

#include <cstdint>
#include <functional>
#include <string>
#include <type_traits>
#include <utility>

#include "lua.hpp"
#include "Core/BellLogging.hpp"
#include "Engine/GeomUtils.h"


#define LUA_SCRIPT_HOOK_NAME(C, F) #C "_" #F

#define LUA_SCRIPT_HOOK_DECLARATION(C, F) int C ## _ ## F(lua_State*);

#define LUA_SCRIPT_HOOK_DEFINITION(C, F) int C ## _ ## F(lua_State* L)  \
{									\
    Tempest::ScriptEngine* se = Tempest::getScriptEngine();				\
    Tempest::ScriptableCallableBase* callable = se->getCallableByName(LUA_SCRIPT_HOOK_NAME(C, F)); \
    return callable->callFunction(L);					\
}

#define LUA_REGISTER_HOOK(C, F, I, ...) \
    { \
        auto* callable = new Tempest::ScriptableCallable<decltype(&C::F), __VA_ARGS__>(&C::F, I); \
        const std::string name = LUA_SCRIPT_HOOK_NAME(C, F); \
        registrar->registerLuaCallable(name, callable); \
        Tempest::s_dispatchFunctions[name] = &C ## _ ## F;      \
    }

namespace Tempest {

    template<typename ...S>
    struct LuaStack {
    };

    inline void setLuaTableEntry(lua_State *L, const std::string &key, const lua_Number val) {
        lua_pushstring(L, key.c_str());
        lua_pushnumber(L, val);
        lua_settable(L, -3);
    }

    inline void setLuaTableEntry(lua_State *L, const std::string &key, const uint32_t val) {
        lua_pushstring(L, key.c_str());
        lua_pushinteger(L, val);
        lua_settable(L, -3);
    }

    template<typename T>
    inline T getTableEntry(lua_State *L, const char *key, const uint32_t i) {
        BELL_TRAP;

        return T{};
    }

    template<>
    inline int getTableEntry(lua_State *L, const char *key, const uint32_t i) {
        lua_pushstring(L, key);
        lua_gettable(L, i);  // get table[key]

        int result = static_cast<int>(lua_tonumber(L, -1));
        lua_pop(L, 1);  // remove number from stack
        return result;
    }

    template<>
    inline float getTableEntry(lua_State *L, const char *key, const uint32_t i) {
        lua_pushstring(L, key);
        lua_gettable(L, i);  // get table[key]

        float result = static_cast<float>(lua_tonumber(L, -1));
        lua_pop(L, 1);  // remove number from stack
        return result;
    }

    template<>
    inline std::string getTableEntry(lua_State *L, const char *key, const uint32_t i) {
        lua_pushstring(L, key);
        lua_gettable(L, i);

        std::string result = lua_tostring(L, -1);
        lua_pop(L, 1);
        return result;
    }

    template<typename T>
    T popLuaStack(lua_State *, uint32_t &) {
        BELL_TRAP;

        return T{};
    }


    template<>
    inline int popLuaStack(lua_State *L, uint32_t &i) {
        int n = static_cast<int>(lua_tointeger(L, i));
        ++i;
        return n;
    }

    template<>
    inline uint32_t popLuaStack(lua_State *L, uint32_t &i) {
        uint32_t n = static_cast<uint32_t>(lua_tointeger(L, i));
        ++i;
        return n;
    }

    template<>
    inline uint64_t popLuaStack(lua_State *L, uint32_t &i) {
        uint64_t n = lua_tointeger(L, i);
        ++i;
        return n;
    }

    template<>
    inline lua_Number popLuaStack(lua_State *L, uint32_t &i) {
        lua_Number n = lua_tonumber(L, i);
        ++i;
        return n;
    }

    template<>
    inline float popLuaStack(lua_State *L, uint32_t &i) {
        float n = static_cast<float>(lua_tonumber(L, i));
        ++i;
        return n;
    }

    template<>
    inline std::string popLuaStack(lua_State *L, uint32_t &i) {
        std::string s = lua_tostring(L, i);
        ++i;
        return s;
    }

    template<>
    inline float3 popLuaStack(lua_State *L, uint32_t &i) {
        float3 v;
        v.x = getTableEntry<float>(L, "x", i);
        v.y = getTableEntry<float>(L, "y", i);
        v.z = getTableEntry<float>(L, "z", i);

        ++i;
        return v;
    }


    inline void pushLuaStack(lua_State *L, const int i) {
        lua_pushinteger(L, i);
    }

    inline void pushLuaStack(lua_State *L, const uint32_t i) {
        lua_pushinteger(L, i);
    }

    inline void pushLuaStack(lua_State *L, const uint64_t i) {
        lua_pushinteger(L, i);
    }

    inline void pushLuaStack(lua_State *L, const lua_Number n) {
        lua_pushnumber(L, n);
    }

    inline void pushLuaStack(lua_State *L, const std::string s) {
        lua_pushstring(L, s.c_str());
    }

    inline void pushLuaStack(lua_State *L, const float3 &f) {
        lua_newtable(L);
        setLuaTableEntry(L, "x", f.x);
        setLuaTableEntry(L, "y", f.y);
        setLuaTableEntry(L, "z", f.z);
    }

    template<typename F, typename I, typename H, typename ...Stack, template<typename...> class S, typename...Args>
    int executeCallback_impl(lua_State *L, F f, I *instance, uint32_t stackDepth, S<H, Stack...>, Args ...args) {
        const auto p = popLuaStack<H>(L, stackDepth);
        return executeCallback_impl(L, f, instance, stackDepth, LuaStack<Stack...>{}, args..., p);
    }

    template<typename F, typename I, template<typename...> class S, typename...Args>
    int executeCallback_impl(lua_State *L, F f, I *instance, uint32_t, S<>, Args ...args) {
        if constexpr (std::is_same_v<typename std::invoke_result_t<F, I, Args...>, void>) {
            std::invoke(f, instance, args...);
            return 0;
        } else {
            const auto result = std::invoke(f, instance, args...);
            pushLuaStack(L, result);
            return 1;
        }
    }


    template<typename F, typename I, typename ...Stack>
    int executeCallback(lua_State *L, F f, I *instance) {
        return executeCallback_impl(L, f, instance, 1u, LuaStack<Stack...>{});
    }

}
#endif
