# This file is part of the MinVR cmake build system.  
# See the main MinVR/CMakeLists.txt file for authors, copyright, and license info.
#
# Builds the MinVR_VisLink plugin as a shared library.

# Note: This CMakeLists.txt is not intended to work on its own.  It inherits many settings 
# from the root CMakeLists.txt (build and install directories, compiling flags, #defines, etc.)



# Plugin names must begin with "MinVR_"
project(MinVR_VisLink)


set(CMAKE_FRAMEWORK_DIR ${CMAKE_SOURCE_DIR}/cmake/framework)

if(NOT EXISTS ${CMAKE_FRAMEWORK_DIR})
execute_process(
    #COMMAND git clone ${PROJECT_SOURCE_DIR}/.. ${CMAKE_FRAMEWORK_DIR}
    COMMAND git clone https://github.com/dtorban/cmake_fw.git ${CMAKE_FRAMEWORK_DIR}
)
endif()

include(${CMAKE_FRAMEWORK_DIR}/project/project.cmake)


# Source:
set (SOURCEFILES 
  src/VRVisLinkTextureNode.cpp
  src/VRVisLinkRenderTextureNode.cpp
  src/VRVisLinkPlugin.cpp
)
set (HEADERFILES
  src/VRVisLinkTextureNode.h
)

#---------------------------- Dependencies ---------------------------

set(VisLinkArgs -DCMAKE_DEBUG_POSTFIX=d)
add_external(VisLinkLib 
  GIT_REPOSITORY https://github.com/dtorban/VisLink.git
  CMAKE_ARGS VisLinkArgs
    GIT_TAG master
  LIB_NAME VisLink
  DEBUG_POSTFIX d
)

set(vislink_external_dir ${external_dir}/VisLinkLib/src/cmake/framework/external)

add_definitions(-DGLM_FORCE_INLINE)
include_directories(${external_dir}/glm/src)
include_directories(${external_dir}/VisLinkLib/src/src)

set(DEP_LINK_DIRS ${DEP_LINK_DIRS} 
  ${vislink_external_dir}/SandBox/build/install/lib
)
set(DEP_INCLUDES ${DEP_INCLUDES} 
  ${vislink_external_dir}/SandBox/src/src
  ${vislink_external_dir}/SandBox/cmake/framework/external/glm/src
  ${vislink_external_dir}/SandBox/src/cmake/framework/external/stb/src
)

set(DEP_LINK_DIRS ${DEP_LINK_DIRS} 
  ${vislink_external_dir}/SandBox/src/cmake/framework/external/glfw3/build/install/lib
)
set(DEP_INCLUDES ${DEP_INCLUDES} 
  ${vislink_external_dir}/SandBox/src/cmake/framework/external/glfw3/build/install/include
)
if(MSVC)
  set(DEP_LIBS ${DEP_LIBS} optimized glfw3.lib sandbox.lib debug glfw3d.lib sandbox.lib)
else()
  set(DEP_LIBS ${DEP_LIBS} optimized libglfw3.a sandbox.a debug libglfw3d.a sandbox.a)
endif()

find_package(Vulkan REQUIRED)
message("vk Found" ${Vulkan_FOUND} ${Vulkan_INCLUDE_DIRS} ${Vulkan_LIBRARIES})
include_directories(${Vulkan_INCLUDE_DIRS})
set(DEP_LIBS ${DEP_LIBS} ${Vulkan_LIBRARIES})

include(${external_dir}/OpenGL/OpenGL.cmake)
useOpenGL()
useGLEW()

#---------------------------------------------------------------------

addSharedLibrary()


# Define the target
#add_library(${PROJECT_NAME} SHARED ${HEADERFILES} ${SOURCEFILES})

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>                  # for headers when building
    $<INSTALL_INTERFACE:${INSTALL_PLUGININCLUDE_DEST}/${PROJECT_NAME}>  # for client in install mode
)


# Setup dependency on libMinVR:
target_include_directories(${PROJECT_NAME} PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../src>  # for headers when building
    $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DEST}>              # for client in install mode
)
target_link_libraries(${PROJECT_NAME} MinVR)



# Setup dependency on OpenGL:
#include(AutoBuildOpenGL)
#AutoBuild_use_package_OpenGL(${PROJECT_NAME} PUBLIC)


# Setup dependency on GLEW (only needed on Windows):
#if (NOT APPLE)
#  include(AutoBuildGLEW)
#  AutoBuild_use_package_GLEW(${PROJECT_NAME} PUBLIC)
#endif()

# Installation:
install(TARGETS ${PROJECT_NAME} EXPORT MinVRTargets COMPONENT ${PROJECT_NAME}
    LIBRARY DESTINATION ${INSTALL_PLUGINLIB_DEST}
    ARCHIVE DESTINATION ${INSTALL_PLUGINLIB_DEST}
    RUNTIME DESTINATION ${INSTALL_PLUGINDLL_DEST}
)
install(FILES ${HEADERFILES} DESTINATION ${INSTALL_PLUGININCLUDE_DEST}/${PROJECT_NAME} COMPONENT ${PROJECT_NAME})


# For better organization when using an IDE with folder structures:
source_group("Header Files" FILES ${HEADERFILES})
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "plugins")

