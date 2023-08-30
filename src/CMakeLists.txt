set(SERENITY_ENGINE_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/include/serenity-engine")

set(SERENITY_ENGINE_SRC_FILES
    "${SERENITY_ENGINE_INCLUDE_PATH}/core/singleton_instance.hpp"

    "${SERENITY_ENGINE_INCLUDE_PATH}/core/log.hpp"
    "core/log.cpp"

    "${SERENITY_ENGINE_INCLUDE_PATH}/core/file_system.hpp"
    "core/file_system.cpp"
    
    "${SERENITY_ENGINE_INCLUDE_PATH}/core/application.hpp"
    "core/application.cpp"
)

add_library(serenity-engine STATIC ${SERENITY_ENGINE_SRC_FILES})
target_precompile_headers(serenity-engine PUBLIC "${CMAKE_SOURCE_DIR}/include/serenity-engine/pch.hpp")
target_compile_definitions(serenity-engine PUBLIC "$<$<CONFIG:DEBUG>:DEF_SERENITY_DEBUG>")
target_include_directories(serenity-engine PUBLIC "${CMAKE_SOURCE_DIR}/include/")
target_link_libraries(serenity-engine PUBLIC external d3d12)
