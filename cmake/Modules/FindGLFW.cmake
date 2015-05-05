#
# Try to find GLFW library and include path.
# Once done this will define
#
# GLFW_FOUND
# GLFW_INCLUDE_DIRS
# GLFW_LIBRARY
#

if(NOT GLFW_FOUND)

FIND_PATH(GLFW_INCLUDE_DIR GLFW/glfw3.h
  PATHS
    ${PROJECT_SOURCE_DIR}/../../external/glfw/include
    ${PROJECT_SOURCE_DIR}/../external/glfw/include
    /usr/local/include
    /usr/X11/include
    /usr/include
    /opt/local/include
    NO_DEFAULT_PATH
    )

FIND_LIBRARY( GLFW_LIBRARY
  NAMES glfw glfw3
  PATHS
    ${PROJECT_SOURCE_DIR}/../../external/glfw/src
    ${PROJECT_SOURCE_DIR}/../external/glfw/src
    ${PROJECT_SOURCE_DIR}/../../external/glfw/lib/x64
    ${PROJECT_SOURCE_DIR}/../external/glfw/lib/x64
    /usr/local
    /usr/X11
    /usr
    PATH_SUFFIXES
    a
    lib64
    lib
    NO_DEFAULT_PATH
)

SET(GLFW_FOUND "NO")
IF (GLFW_INCLUDE_DIR AND GLFW_LIBRARY)
  SET(GLFW_FOUND "YES")
  SET(GLFW_INCLUDE_DIRS ${GLFW_INCLUDE_DIR})
ENDIF (GLFW_INCLUDE_DIR AND GLFW_LIBRARY)

if(GLFW_FOUND)
  message(STATUS "Found GLFW: ${GLFW_INCLUDE_DIR}")
else(GLFW_FOUND)
  message(FATAL_ERROR "could NOT find GLFW")
endif(GLFW_FOUND)

endif(NOT GLFW_FOUND)