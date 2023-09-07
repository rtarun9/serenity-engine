include(FetchContent)

# SDL3 for input / window creation and handling.
FetchContent_Declare(
    SDL3
	GIT_REPOSITORY https://github.com/libsdl-org/SDL
	GIT_TAG 2471d8cc2ac07511e60c062748ed1952bd18144e # The latest commit as of writing this cmake script.
	GIT_PROGRESS TRUE
)

# spdlog for logging to console and file.
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.10.0
    GIT_PROGRESS   TRUE
)

# fastgltf for GLTF asset loading.
FetchContent_Declare(
    fastgltf
    GIT_REPOSITORY https://github.com/spnda/fastgltf.git
    GIT_TAG        b857a186c21262d0749ab31f5a2954f02725f47c
)

# stb for image loading.
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb
    GIT_TAG 8b5f1f37b5b75829fc72d38e7b5d4bcbf8a26d55
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(SDL3 spdlog fastgltf stb)

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

add_library(external INTERFACE)
target_link_libraries(external INTERFACE SDL3::SDL3 libimgui fastgltf)	
target_include_directories(external INTERFACE ${sdl3_SOURCE_DIR}/include ${spdlog_SOURCE_DIR}/include ${stb_SOURCE_DIR}) 