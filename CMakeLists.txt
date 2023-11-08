cmake_minimum_required(VERSION 3.8)
project(tinycar_runtime C CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O3")

set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

set(OpenGL_GL_PREFERENCE "LEGACY")
find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenCV REQUIRED)

# Dear ImGui
set(IMGUI_DIR ./imgui)
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends ..)
set(NV_DIR ./nv)

set(SOURCES
main.cpp
${NV_DIR}/nv.cpp
)

set(IMGUI_SOURCES
${IMGUI_DIR}/imgui.cpp
${IMGUI_DIR}/imgui_demo.cpp
${IMGUI_DIR}/imgui_draw.cpp
${IMGUI_DIR}/imgui_tables.cpp
${IMGUI_DIR}/imgui_widgets.cpp
${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

add_executable(tinycar_runtime ${SOURCES} ${IMGUI_SOURCES})
target_link_libraries(tinycar_runtime PUBLIC ${OpenCV_LIBS} glfw OpenGL::GL ${CMAKE_DL_LIBS})
target_compile_features(tinycar_runtime PRIVATE cxx_std_17)
target_include_directories(tinycar_runtime PUBLIC ${NV_DIR})