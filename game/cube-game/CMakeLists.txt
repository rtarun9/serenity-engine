add_executable(game "game.cpp")
target_link_libraries(game PRIVATE serenity-engine)

# Set the Visual studio debugger working directory.
set_property(TARGET serenity-engine PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")