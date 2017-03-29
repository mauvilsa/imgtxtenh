cmake_minimum_required( VERSION 2.8.4 )
project( imgtxtenh )
set( tool_EXE imgtxtenh )
include( FindPkgConfig )
pkg_check_modules( MagickCore REQUIRED MagickCore>=6.8 )

if( MagickCore_VERSION VERSION_GREATER 7.0.0 )
  message( FATAL_ERROR "MagickCore 7 or newer not supported!" )
endif()

file( GLOB tool_SRC "*.c" )
add_executable( ${tool_EXE} ${tool_SRC} )

set( cfs "-Wall -W" )
foreach( cf ${MagickCore_CFLAGS} )
  set( cfs "${cfs} ${cf}" )
endforeach()
set_target_properties( ${tool_EXE} PROPERTIES COMPILE_FLAGS "${cfs}" )

target_link_libraries( ${tool_EXE} ${MagickCore_LDFLAGS} m )

install( TARGETS ${tool_EXE} DESTINATION bin )

add_custom_target( realclean cd ${CMAKE_HOME_DIRECTORY} COMMAND rm -fr ${tool_EXE} ${tool_EXE}.exe CMakeFiles CMakeCache.txt cmake_install.cmake install_manifest.txt Makefile )
