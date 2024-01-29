cmake_minimum_required(VERSION 3.26)

if (APPLE) 
###################
# FOR SWIFT BACKENDS
###################
    project(tinycar_runtime LANGUAGES C CXX Swift)

    if("${CMAKE_Swift_COMPILER_VERSION}" VERSION_LESS 5.9)
        message(FATAL_ERROR "Bidirectional C++ Interop requires Swift 5.9 or greater. Have ${CMAKE_Swift_COMPILER_VERSION}")
    endif()

    if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND
    NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
        message(FATAL_ERROR "Project requires building with Clang.
        Have ${CMAKE_CXX_COMPILER_ID}")
    endif()

    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/swift")

    include(InitializeSwift)
    # cmake/modules/AddSwift.cmake provides the function for creating the Swift to
    # C++ bridging header
    include(AddSwift)

    set(CMAKE_OSX_DEPLOYMENT_TARGET 13.0)

    # After changing Swift interface code, we need to re-run the Swift compiler to generate c++ bridging header
    set_property(DIRECTORY PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
   "${CMAKE_CURRENT_SOURCE_DIR}/src/backends/nn/coreml_backend_swift/coreml_backend.swift")

    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/backends/nn/coreml_backend_swift")
else()
###################
# NON APPLE
###################
    project(tinycar_runtime LANGUAGES C CXX)
endif()

##############
# project needs additonal content like debug images/videos and neural network models
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/additional")
include(ContentDownload)
# Set the output directory relative to the current source directory
set(TARGET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/debug_files")
# Call the function with the target directory and download URL
download_and_unzip("${TARGET_DIR}" "http://riege.com.de/tinycar_runtime/debug_files.zip")

set(CMAKE_CXX_STANDARD 17)

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

include_directories(./src)

file(GLOB SRC_SOURCES src/*.cpp src/viewcontroller/*.cpp)

# Tinycar
set(TINYCAR_DIR ./tinycar_lib)
include_directories(${TINYCAR_DIR})
file(GLOB TINYCAR_SOURCES ${TINYCAR_DIR}/*.cpp)

set(SOURCES
main.cpp
${NV_DIR}/nv.cpp
${SRC_SOURCES}
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

add_executable(tinycar_runtime ${SOURCES} ${IMGUI_SOURCES} ${TINYCAR_SOURCES})

if (APPLE) 
  target_link_libraries(tinycar_runtime PUBLIC ${OpenCV_LIBS} glfw OpenGL::GL ${CMAKE_DL_LIBS} coreml_backend_swift)
else()
  target_link_libraries(tinycar_runtime PUBLIC ${OpenCV_LIBS} glfw OpenGL::GL ${CMAKE_DL_LIBS})
endif()

target_compile_features(tinycar_runtime PRIVATE cxx_std_17)
target_include_directories(tinycar_runtime PUBLIC ${NV_DIR})