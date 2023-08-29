set(SERENITY_ENGINE_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/include/serenity-engine")

set(SERENITY_ENGINE_SRC_FILES
    "main.cpp"

    "${SERENITY_ENGINE_INCLUDE_PATH}/core/log.hpp"
    "core/log.cpp"

    "${SERENITY_ENGINE_INCLUDE_PATH}/core/file_system.hpp"
    "core/file_system.cpp"
)

find_package(Vulkan REQUIRED)

add_executable(serenity-engine ${SERENITY_ENGINE_SRC_FILES})
target_precompile_headers(serenity-engine PUBLIC "${CMAKE_SOURCE_DIR}/include/serenity-engine/pch.hpp")
target_compile_definitions(serenity-engine PUBLIC "$<$<CONFIG:DEBUG>:DEF_SERENITY_DEBUG>")
target_include_directories(serenity-engine PUBLIC "${CMAKE_SOURCE_DIR}/include/")
target_link_libraries(serenity-engine PUBLIC external d3d12)


# Set the Visual studio debugger working directory.
set_property(TARGET serenity-engine PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
