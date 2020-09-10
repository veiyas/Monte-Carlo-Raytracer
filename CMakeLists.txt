cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project("MC Raytracer")
#
# Add the libraries that this application depends on


# Adding the source files here that are compiled for this project
add_executable(${PROJECT_NAME}
  src/main.cpp
  src/basic_types.hpp
  src/basic_types.cpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw glad)
#
# Setting some compile settings for the project
#
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
if (MSVC)
  # Microsoft Visual Studio related compile options
  target_compile_options(${PROJECT_NAME} PRIVATE
    "/ZI"       # Edit and continue support
    "/MP"       # Multi-threading support
    "/W4"       # Highest warning level
    "/wd4201"   # nonstandard extension used : nameless struct/union    
    "/std:c++17"
    "/permissive-"
    "/Zc:strictStrings-"    # Windows header don't adhere to this
    "/Zc:__cplusplus" # Correctly set the __cplusplus macro
  )
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # When compiling on Clang.  This most likely means compiling on MacOS
  target_compile_options(${PROJECT_NAME} PRIVATE
    "-stdlib=libc++"
    "-Wall"
    "-Wextra"
  )
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  # Probably compiling on Linux
  target_compile_options(${PROJECT_NAME} PRIVATE
    "-ggdb"
    "-Wall"
    "-Wextra"
    "-Wpedantic"
  )
endif ()
