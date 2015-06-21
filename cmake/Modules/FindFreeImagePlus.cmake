#
# Try to find the FreeImage library and include path.
# Once done this will define
#
# FREEIMAGEPLUS_FOUND
# FREEIMAGEPLUS_INCLUDE_PATH
# FREEIMAGEPLUS_LIBRARY
# FREEIMAGEPLUS_LIBRARIES
# 

IF (WIN32)
	FIND_PATH( FREEIMAGEPLUS_INCLUDE_PATH FreeImagePlus.h
		DOC "The directory where FreeImagePlus.h resides")
	FIND_LIBRARY( FREEIMAGEPLUS_LIBRARY
		NAMES FreeImagePlus freeimageplus
		DOC "The FreeImagePlus library")
ELSE (WIN32)
	FIND_PATH( FREEIMAGEPLUS_INCLUDE_PATH FreeImagePlus.h
		/usr/include
		/usr/local/include
		/sw/include
		/opt/local/include
		DOC "The directory where FreeImagePlus.h resides")
	FIND_LIBRARY( FREEIMAGEPLUS_LIBRARY
		NAMES FreeImagePlus freeimageplus
		PATHS
		/usr/lib64
		/usr/lib
		/usr/local/lib64
		/usr/local/lib
		/sw/lib
		/opt/local/lib
		DOC "The FreeImagePlus library")
ENDIF (WIN32)

SET(FREEIMAGEPLUS_LIBRARIES ${FREEIMAGEPLUS_LIBRARY})

IF (FREEIMAGEPLUS_INCLUDE_PATH AND FREEIMAGEPLUS_LIBRARY)
	SET( FREEIMAGE_INCLUDE_DIRS ${FREEIMAGE_INCLUDE_PATH})
	SET( FREEIMAGEPLUS_FOUND "YES")
ELSE (FREEIMAGEPLUS_INCLUDE_PATH AND FREEIMAGEPLUS_LIBRARY)
	SET( FREEIMAGEPLUS_FOUND "NO")
ENDIF (FREEIMAGEPLUS_INCLUDE_PATH AND FREEIMAGEPLUS_LIBRARY)
