cmake_minimum_required (VERSION 3.10)    
project (TEMPEST)    
    
    
set(CMAKE_CXX_STANDARD 17)    
    
set(BELL_PATH "../Bell")    
    
set(BELL_SHADER_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/)
add_subdirectory(${BELL_PATH} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../Bellbuilds/${CMAKE_BUILD_TYPE})
add_subdirectory(Source/jsoncpp)
add_subdirectory(Source/bullet3)

include_directories(	"${BELL_PATH}"
    "${BELL_PATH}/Include"
    "${BELL_PATH}/Source"
    "${BELL_PATH}/ThirdParty/GLFW3/glfw-3.2.1/include"
    "${BELL_PATH}/ThirdParty/assimp-4.1.0/include"
    "${BELL_PATH}/ThirdParty/imgui-master"
    "${BELL_PATH}/ThirdParty/glslang"
    "${BELL_PATH}/ThirdParty/imgui-node-editor/NodeEditor/Include"
    "${BELL_PATH}/ThirdParty/glm"
	"${BELL_PATH}/ThirdParty/imgui-guizmo"
	"${BELL_PATH}/ThirdParty/stb_image"
	"Source/lua/src"
	"Source/bullet3/src/"
	"Source/jsoncpp/include"
	"Source/Physics"
	"Source/Graphics"
	"Source/GamePlay"
	"Source/Scripting")

file(COPY "${CMAKE_CURRENT_LIST_DIR}/Assets" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/")

set(LUA_SOURCE
    Source/lua/src/lapi.c
    Source/lua/src/lcode.c
    Source/lua/src/lctype.c
    Source/lua/src/ldebug.c
    Source/lua/src/ldo.c
    Source/lua/src/ldump.c
    Source/lua/src/lfunc.c
    Source/lua/src/lgc.c
    Source/lua/src/llex.c
    Source/lua/src/lmem.c
    Source/lua/src/lobject.c
    Source/lua/src/lopcodes.c
    Source/lua/src/lparser.c
    Source/lua/src/lstate.c
    Source/lua/src/lstring.c
    Source/lua/src/ltable.c
    Source/lua/src/ltm.c
    Source/lua/src/lundump.c
    Source/lua/src/lvm.c
    Source/lua/src/lzio.c
    Source/lua/src/lauxlib.c
    Source/lua/src/lbaselib.c
    Source/lua/src/lcorolib.c
    Source/lua/src/ldblib.c
    Source/lua/src/liolib.c
    Source/lua/src/lmathlib.c
    Source/lua/src/loadlib.c
    Source/lua/src/loslib.c
    Source/lua/src/lstrlib.c
    Source/lua/src/ltablib.c
    Source/lua/src/lutf8lib.c
    Source/lua/src/linit.c)

set(ENGINE_SOURCE
    Source/Graphics/RenderThread.cpp
    Source/Scripting/ScriptEngine.cpp
    Source/Level.cpp
    Source/TempestEngine.cpp
    Source/Physics/PhysicsWorld.cpp
    Source/GamePlay/NavMesh.cpp
	Source/GamePlay/ScriptEventQueue.cpp
	Source/GamePlay/Controller.cpp
	Source/GamePlay/Player.cpp
    Source/Scripting/ScriptableScene.cpp
	Source/Scripting/ScriptableEngine.cpp
	Source/Scripting/ScriptableRenderer.cpp)

set(BULLET
    BulletInverseDynamics
    BulletDynamics
    BulletSoftBody
    BulletCollision
    Bullet3Common
    LinearMath
    )

set(TEMPEST_EDITOR_SOURCE
		Source/Editor/TempestEditor.cpp
		Source/Editor/SceneWindow.cpp
		Source/Editor/InstanceWindow.cpp
		Source/Editor/GraphicsSettingsWindow.cpp)

add_executable(TEMPEST ${ENGINE_SOURCE} Source/main.cpp ${TEMPEST_EDITOR_SOURCE} ${LUA_SOURCE})
target_link_libraries(TEMPEST BELL jsoncpp_static ${BULLET})

add_executable(TEMPEST_EDITOR ${ENGINE_SOURCE} Source/Editor/main.cpp ${TEMPEST_EDITOR_SOURCE} ${LUA_SOURCE})
target_link_libraries(TEMPEST_EDITOR BELL jsoncpp_static ${BULLET})

add_dependencies(TEMPEST_EDITOR TEMPEST)
