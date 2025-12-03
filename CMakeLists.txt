cmake_minimum_required (VERSION 3.0)
cmake_policy(VERSION 3.0)

project(Template)

set(CMAKE_OSX_ARCHITECTURES "arm64")

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
    CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_definitions(/std:c++17)
endif()

set(CMAKE_CXX_EXTENSIONS ON)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
link_directories(${CMAKE_SOURCE_DIR}/lib)

# find the required packages
find_package(GLM REQUIRED)
message(STATUS "GLM included at ${GLM_INCLUDE_DIR}")
find_package(GLFW3 REQUIRED)
message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")
find_package(ASSIMP REQUIRED)
message(STATUS "Found ASSIMP in ${ASSIMP_INCLUDE_DIR}")

find_package(Freetype REQUIRED)
message(STATUS "Found Freetype in ${FREETYPE_INCLUDE_DIRS}")

# Find Bullet Physics
find_package(Bullet REQUIRED)
message(STATUS "Found Bullet in ${BULLET_INCLUDE_DIR}")
include_directories(${BULLET_INCLUDE_DIR})

INCLUDE_DIRECTORIES(/System/Library/Frameworks)
FIND_LIBRARY(COCOA_LIBRARY Cocoa)
FIND_LIBRARY(OpenGL_LIBRARY OpenGL)
FIND_LIBRARY(IOKit_LIBRARY IOKit)
FIND_LIBRARY(CoreVideo_LIBRARY CoreVideo)
MARK_AS_ADVANCED(COCOA_LIBRARY OpenGL_LIBRARY)
SET(APPLE_LIBS ${COCOA_LIBRARY} ${IOKit_LIBRARY} ${OpenGL_LIBRARY} ${CoreVideo_LIBRARY})
SET(APPLE_LIBS ${APPLE_LIBS} ${GLFW3_LIBRARY} ${ASSIMP_LIBRARY} ${FREETYPE_LIBRARIES})
set(LIBS ${LIBS} ${APPLE_LIBS} ${BULLET_LIBRARIES})

# Set Project to build
set(PROJECTS
  game_project
)

configure_file(configuration/root_directory.h.in configuration/root_directory.h)
include_directories(${CMAKE_BINARY_DIR}/configuration)

add_library(STB_IMAGE "src/stb_image.cpp")
set(LIBS ${LIBS} STB_IMAGE)


add_library(GLAD "src/glad.c")
set(LIBS ${LIBS} GLAD)

macro(makeLink src dest target)
  add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E create_symlink ${src} ${dest}  DEPENDS  ${dest} COMMENT "mklink ${src} -> ${dest}")
endmacro()

function(create_project_from_sources chapter)
      file(GLOB_RECURSE SOURCE
        "src/${chapter}/*.h"
        "src/${chapter}/*.cpp"
        "src/${chapter}/*.vs"
        "src/${chapter}/*.fs"
        "src/${chapter}/*.tcs"
        "src/${chapter}/*.tes"
        "src/${chapter}/*.gs"
        "src/${chapter}/*.cs"
      )
	set(NAME "${chapter}")
  add_executable(${NAME} ${SOURCE})
  target_link_libraries(${NAME} ${LIBS})
  if(MSVC)
  target_compile_options(${NAME} PRIVATE /std:c++17 /MP)
      target_link_options(${NAME} PUBLIC /ignore:4099)
  endif(MSVC)
  set_target_properties(${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${chapter}")
  set_target_properties(${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin/${chapter}")
  set_target_properties(${NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/${chapter}")
  # copy shader files to build directory
  file(GLOB SHADERS
            "src/${chapter}/*.vs"
            "src/${chapter}/*.fs"
  )
# copy dlls
file(GLOB DLLS "dlls/*.dll")
  foreach(SHADER ${SHADERS})
    get_filename_component(SHADERNAME ${SHADER} NAME)
    makeLink(${SHADER} ${CMAKE_SOURCE_DIR}/bin/${chapter}/${SHADERNAME} ${NAME})
  endforeach(SHADER)
  if(MSVC)
      configure_file(${CMAKE_SOURCE_DIR}/configuration/visualstudio.vcxproj.user.in ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.vcxproj.user @ONLY)
  endif(MSVC)
endfunction()

# then create a project file per tutorial
foreach(PROJECTS ${PROJECTS})
  create_project_from_sources(${PROJECTS})
endforeach(PROJECTS)

include_directories(${CMAKE_SOURCE_DIR}/includes)
