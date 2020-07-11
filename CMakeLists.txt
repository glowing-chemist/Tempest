cmake_minimum_required (VERSION 3.10)    
project (TEMPEST)    
    
    
set(CMAKE_CXX_STANDARD 17)    
    
set(BELL_PATH "../Bell")    
    
add_subdirectory(${BELL_PATH} ${CMAKE_CURRENT_BINARY_DIR}/Bell)
    
include_directories(	"${BELL_PATH}"
    "${BELL_PATH}/Include"
            		"${BELL_PATH}/Source"    
        		"${BELL_PATH}/ThirdParty/GLFW3/glfw-3.2.1/include"    
                        "${BELL_PATH}/ThirdParty/assimp-4.1.0/include"    
                        "${BELL_PATH}/ThirdParty/imgui-master"    
                        "${BELL_PATH}/ThirdParty/glslang"    
                        "${BELL_PATH}/ThirdParty/imgui-node-editor/NodeEditor/Include"    
                    	"${BELL_PATH}/ThirdParty/glm"           
			"${BELL_PATH}/ThirdParty/stb_image")

file(COPY "${CMAKE_CURRENT_LIST_DIR}/Assets" DESTINATION "${CMAKE_BINARY_DIR}/")


add_executable(TEMPEST Source/main.cpp Source/Player.cpp Source/Controller.cpp Source/RenderThread.cpp)
target_link_libraries(TEMPEST BELL) 
