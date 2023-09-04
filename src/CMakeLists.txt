set(SERENITY_ENGINE_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/include/serenity-engine")

add_library(serenity-engine)

add_subdirectory(core)
add_subdirectory(graphics)
add_subdirectory(utils)
add_subdirectory(window)
add_subdirectory(scene)
add_subdirectory(asset)
add_subdirectory(main)

target_precompile_headers(serenity-engine PUBLIC "${PROJECT_SOURCE_DIR}/include/serenity-engine/pch.hpp")
target_compile_definitions(serenity-engine PUBLIC "$<$<CONFIG:DEBUG>:DEF_SERENITY_DEBUG>")
target_include_directories(serenity-engine PUBLIC "${PROJECT_SOURCE_DIR}/include/" "${PROJECT_SOURCE_DIR}/" PRIVATE "${PROJECT_SOURCE_DIR}/include/serenity-engine/")
target_link_libraries(serenity-engine PUBLIC external d3d12 dxgi dxguid dxcompiler)