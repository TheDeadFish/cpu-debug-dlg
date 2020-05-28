project(sid-play)
cmake_minimum_required(VERSION 3.12)
set( EXECUTABLE_OUTPUT_PATH ../bin)
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../bin)

#add_link_options(-s -Wl,--gc-sections -mwindows)

#link_libraries(-lwin32hlp -lvgxemu -lstdshit)

add_library(cpu-debug STATIC
	cpu-break.cc
	cpu-debug-res.rc
	cpu-debug.cc
)

add_custom_command(TARGET cpu-debug POST_BUILD COMMAND 
	libres $<TARGET_FILE:cpu-debug> $<TARGET_FILE:cpu-debug>)

add_executable(test
	test.cc
)

target_link_libraries(test cpu-debug -lwin32hlp -lstdshit)
