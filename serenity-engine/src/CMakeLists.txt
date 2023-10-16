set(SERENITY_ENGINE_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/serenity-engine/include/serenity-engine")

add_library(serenity-engine)

add_subdirectory(core)
add_subdirectory(renderer)
add_subdirectory(utils)
add_subdirectory(window)
add_subdirectory(scene)
add_subdirectory(asset)
add_subdirectory(editor)
add_subdirectory(main)
add_subdirectory(scripting)

target_precompile_headers(serenity-engine PUBLIC "${SERENITY_ENGINE_INCLUDE_PATH}/pch.hpp")
target_compile_definitions(serenity-engine PUBLIC "$<$<CONFIG:DEBUG>:DEF_SERENITY_DEBUG>")
target_include_directories(serenity-engine PUBLIC "${PROJECT_SOURCE_DIR}/serenity-engine/include/" "${CMAKE_SOURCE_DIR}/" PRIVATE "${SERENITY_ENGINE_INCLUDE_PATH}")
target_link_libraries(serenity-engine PUBLIC external d3d12 dxgi dxguid dxcompiler)
target_sources(serenity-engine PUBLIC "${SERENITY_ENGINE_INCLUDE_PATH}/serenity-engine.hpp")