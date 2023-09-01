include(FetchContent)

FetchContent_Declare(
    SDL3
	GIT_REPOSITORY https://github.com/libsdl-org/SDL
	GIT_TAG 2471d8cc2ac07511e60c062748ed1952bd18144e # The latest commit as of writing this cmake script.
	GIT_PROGRESS TRUE
)

FetchContent_Declare(
    glm
	GIT_REPOSITORY https://github.com/g-truc/glm
	GIT_TAG 47585fde0c49fa77a2bf2fb1d2ead06999fd4b6e # The latest commit as of writing this cmake script.
	GIT_PROGRESS TRUE
)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.10.0
    GIT_PROGRESS   TRUE
)

FetchContent_MakeAvailable(SDL3 glm spdlog)

add_library(external INTERFACE)
target_link_libraries(external INTERFACE SDL3::SDL3)	
target_include_directories(external INTERFACE ${sdl3_SOURCE_DIR}/include ${glm_SOURCE_DIR} ${spdlog_SOURCE_DIR}/include) 