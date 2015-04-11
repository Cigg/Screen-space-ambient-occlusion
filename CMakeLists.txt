cmake_minimum_required(VERSION 2.8)

# Set project name
set(PROJECT_NAME Test)
project(${PROJECT_NAME})

# Add path to custom modules (GLFW and GLEW)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/Modules)

# Find external packages
find_package(OPENGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLFW REQUIRED)
find_package(GLM REQUIRED)

# Include GLEW and GLFW directories
include_directories(${GLEW_INCLUDE_DIRS})
include_directories(${GLFW_INCLUDE_DIRS})
include_directories(${GLM_INCLUDE_DIRS})

set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
include_directories(${INCLUDE_DIR})

# set(CMAKE_CXX_FLAGS "-g")

add_executable(Test src/main.cpp)

set_target_properties(Test PROPERTIES COMPILE_FLAGS "-std=c++11")

# Link all libraries to project
target_link_libraries(Test ${OPENGL_LIBRARIES} ${OPENGL_glu_LIBRARY} ${GLEW_LIBRARY} ${GLFW_LIBRARY})

