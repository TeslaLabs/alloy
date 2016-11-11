cmake_minimum_required (VERSION 2.8.3)

project("Alloy")

if(NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/src")
  message(FATAL_ERROR "The Alloy dependency repositories (GLFW, etc.) are missing! "
    "You probably did not clone the project with --recursive. It is possible to recover "
    "by calling \"git submodule update --init --recursive\"")
endif()
option(ALLOY_BUILD_EXAMPLE "Build Alloy Examples?" ON)

set(ALLOY_EXTRA_LIBS "")
set(LIBALLOY_EXTRA_SOURCE "")

include(CheckCXXCompilerFlag)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()
string(TOUPPER "${CMAKE_BUILD_TYPE}" U_CMAKE_BUILD_TYPE)

set(MULTITHREADED_BUILD 8 CACHE STRING "How many threads are used to build the project")
set(BUILD_SHARED_LIBS OFF CACHE BOOL " " FORCE)

if(NOT APPLE)
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL " " FORCE)
  set(GLFW_BUILD_TESTS OFF CACHE BOOL " " FORCE)
  set(GLFW_BUILD_DOCS OFF CACHE BOOL " " FORCE)
  set(GLFW_BUILD_INSTALL OFF CACHE BOOL " " FORCE)
  set(GLFW_INSTALL OFF CACHE BOOL " " FORCE)
  add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw" "ext_build/glfw")
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # Quench annoying deprecation warnings when compiling GLFW on OSX
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-declarations -Wmissing-braces -Wunknown-pragmas")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations -Wmissing-braces -Wunknown-pragmas")
endif()

if (MSVC)
  # Disable annoying MSVC warnings (all targets)
  add_definitions(/D "_CRT_SECURE_NO_WARNINGS")
  add_definitions(/D "NOMINMAX")
  add_definitions(/D "NDEBUG")
  add_definitions(/D "_CONSOLE")
  add_definitions(/D "GLEW_STATIC")
  add_definitions(/D "UNICODE")
  add_definitions(/D "_UNICODE")

  # Parallel build on MSVC (all targets)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /Oi /Gy /Ob2 /W3 /openmp")
endif()

# Compile with compiler warnings turned on
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -DGL_GLEXT_PROTOTYPES=1 -O3 -Wall -fmessage-length=0 -MMD -MP")

  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DGL_GLEXT_PROTOTYPES=1 -O3 -Wall -fmessage-length=0 -MMD -MP")
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
   set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
  endif()
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()


if(WIN32)
  # Build and include GLEW on Windows
  list(APPEND LIBALLOY_EXTRA_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/ext/glew/src/glew.c")
  set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/ext/glew/src/glew.c" PROPERTIES COMPILE_DEFINITIONS GLEW_BUILD)
  include_directories(ext/glew/include)
endif()

# Required core libraries on various platforms
if (WIN32)
  set(GLFW_LIBRARIES "opengl32" CACHE STRING " " FORCE)
  list(APPEND ALLOY_EXTRA_LIBS opengl32 glu32)
elseif (APPLE)
  find_library(cocoa_library Cocoa)
  find_library(opengl_library OpenGL)
  find_library(corevideo_library CoreVideo)
  find_library(iokit_library IOKit)
  find_library(gl_library OpenGL)
  list(APPEND ALLOY_EXTRA_LIBS GLEW glfw3 ${gl_library} ${cocoa_library} ${opengl_library} ${corevideo_library} ${iokit_library})
elseif(CMAKE_SYSTEM MATCHES "Linux")
  list(APPEND ALLOY_EXTRA_LIBS gomp GL Xext GLU GLEW Xi Xrandr X11 Xxf86vm Xinerama Xcursor Xdamage pthread m dl stdc++)
endif()

include_directories(include/core include/examples ext/glfw/include ext/glew/include ${CMAKE_CURRENT_BINARY_DIR})
# Compile main Alloy library 


file(GLOB lib_includes include/core/*.h include/core/cereal/*.h include/core/libkdtree/*h)
file(GLOB lib_files src/core/*.cpp src/core/*.c)
file(GLOB ex_files src/example/*.cpp)
file(GLOB ex_includes include/example/*.h)
add_library(alloy STATIC ${lib_includes} ${lib_files} ${LIBALLOY_EXTRA_SOURCE})

if(ALLOY_BUILD_EXAMPLE)
  add_executable(examples ${ex_includes} ${ex_files})
  include_directories(include/core)
  if (APPLE)
    target_link_libraries(examples alloy ${ALLOY_EXTRA_LIBS})
  else()
    target_link_libraries(examples alloy glfw ${ALLOY_EXTRA_LIBS})
  endif()
endif()
