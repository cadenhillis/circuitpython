# Create an INTERFACE library for our C module.
add_library(usermod_SPA INTERFACE)

# Add our source files to the lib
target_sources(usermod_Igrf INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/SPA.c
    ${CMAKE_CURRENT_LIST_DIR}/SunPos.c
)

# Add the current directory as an include directory.
target_include_directories(usermod_SPA INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Link our INTERFACE library to the usermod target.
target_link_libraries(usermod INTERFACE usermod_SPA)
