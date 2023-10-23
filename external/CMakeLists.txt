include(FetchContent)
include(ExternalProject)

# SDL3 for input / window creation and handling.
FetchContent_Declare(
    SDL3
	GIT_REPOSITORY https://github.com/libsdl-org/SDL
	GIT_TAG 2471d8cc2ac07511e60c062748ed1952bd18144e 
	GIT_PROGRESS TRUE
)

# spdlog for logging to console and file.
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.10.0
    GIT_PROGRESS TRUE
)

# fastgltf for GLTF asset loading.
FetchContent_Declare(
    fastgltf
    GIT_REPOSITORY https://github.com/spnda/fastgltf.git
    GIT_TAG b857a186c21262d0749ab31f5a2954f02725f47c
    GIT_PROGRESS TRUE
)

# stb for image loading.
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb
    GIT_TAG 8b5f1f37b5b75829fc72d38e7b5d4bcbf8a26d55
    GIT_PROGRESS TRUE
)

# sol2 for lua scripting (lua is not downloaded and setup by sol2, so that is done manually).
set(SOL2_BUILD_LUA TRUE)
FetchContent_Declare(
    sol2
    GIT_REPOSITORY https://github.com/ThePhD/sol2
    GIT_TAG 9c882a28fdb6f4ad79a53a4191b43ce48a661175
    GIT_PROGRESS TRUE
)

# Lua for ... lua (sol2 does not setup lua, hence why it is required to be setup manually)
FetchContent_Declare(
    lua
    GIT_REPOSITORY https://github.com/walterschell/Lua
    GIT_TAG 88246d621abf7b6fba9332f49229d507f020e450
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(SDL3 spdlog fastgltf stb sol2 lua)

# imgui for the editor ui.
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG 475453668d7da1662bf90266c5f0df0259705987
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(imgui)

add_library(libimgui
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx12.cpp
)

target_include_directories(libimgui PUBLIC 
    ${imgui_SOURCE_DIR} 
    ${imgui_SOURCE_DIR}/backends
    ${sdl3_SOURCE_DIR}/include
)

target_link_libraries(libimgui PUBLIC SDL3::SDL3)

# bullet3 for physics
FetchContent_Declare(
  bullet
  GIT_REPOSITORY https://github.com/bulletphysics/bullet3
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
  GIT_TAG        master)

set(BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(BUILD_PYBULLET OFF CACHE BOOL "" FORCE)
set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_CPU_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_OPENGL3_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_OPENGL3_DEMOS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(bullet)

add_library(external INTERFACE)
target_link_libraries(external INTERFACE SDL3::SDL3 libimgui fastgltf sol2::sol2 lua_static ${BULLET_LIBRARIES})	
target_include_directories(external INTERFACE ${sdl3_SOURCE_DIR}/include ${spdlog_SOURCE_DIR}/include ${stb_SOURCE_DIR} ${sol2_SOURCE_DIR}/include ${bullet_SOURCE_DIR})  