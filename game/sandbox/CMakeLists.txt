add_executable(sandbox "sandbox.cpp")
target_link_libraries(sandbox PRIVATE serenity-engine)

# Set the Visual studio debugger working directory.
set_property(TARGET serenity-engine PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")