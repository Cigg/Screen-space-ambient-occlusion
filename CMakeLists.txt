cmake_minimum_required(VERSION 2.8)

# Set project name
set(PROJECT_NAME Test)
project(${PROJECT_NAME})

# Add path to custom modules (GLFW and GLEW)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/Modules)

# Find external packages
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLM REQUIRED)

# Find GLFW
if (WIN32)
	# Using FindGLFW.cmake
	# It will set ${GLFW_INCLUDE_DIRS} and ${GLFW_LIBRARY}
	find_package(GLFW REQUIRED)
	set(GLFW_STATIC_LIBRARIES ${OPENGL_LIBRARY} ${GLFW_LIBRARY})
else()
	# Let pkg-config find all dependencies for X11 libs
	# It will set ${GLFW_INCLUDE_DIRS}, ${GLFW_LIBRARIES} and ${GLFW_STATIC_LIBRARIES}
	find_package(PkgConfig REQUIRED)
	pkg_search_module(GLFW REQUIRED glfw3)
 endif()

# Include GLEW, GLFW, GLM and custom directories
include_directories(${GLEW_INCLUDE_DIRS})
include_directories(${GLFW_INCLUDE_DIRS})
include_directories(${GLM_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/include)

# Add all headers and source files
file(GLOB SOURCES "src/*.cpp" "include/*.h")

# Add executable
add_executable(Test ${SOURCES})

# Set compiler flags
set(CMAKE_CXX_FLAGS "-std=c++11")
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.8)		    
		# Use GCC 4.9 color diagnostics
		set(CMAKE_CXX_FLAGS "-std=c++11 -fdiagnostics-color=auto")
	endif()
endif()

set_target_properties(Test PROPERTIES COMPILE_FLAGS "-std=c++11")

# Link all libraries to project
target_link_libraries(Test ${GLEW_LIBRARY} ${OPENGL_LIBRARIES} ${GLFW_STATIC_LIBRARIES})

