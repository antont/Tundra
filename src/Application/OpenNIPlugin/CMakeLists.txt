# Define target name and output directory
#init_target (OpenNIPlugin OUTPUT plugins)

# openNI lib
FIND_LIBRARY(OPEN_NI_LIBRARY NAMES OpenNI PATHS ${OPEN_NI_LIB})

IF(NOT OPEN_NI_LIBRARY)
#IF(NOT OPEN_NI_LIB)
SET(OPEN_NI_LIB $ENV{OPEN_NI_LIB})
IF(NOT DEFINED OPEN_NI_LIB)
# env is not set
MESSAGE("OPEN_NI_LIB should be set, for example:")
MESSAGE("> cmake -DOPEN_NI_LIB=/usr/lib/??? ..")
ELSE()
FIND_LIBRARY(OPEN_NI_LIBRARY NAMES OpenNI PATHS ${OPEN_NI_LIB})
ENDIF()
#ENDIF()
ENDIF()

SET(LIBS ${LIBS} ${OPEN_NI_LIBRARY})


# IF(DEFINED OPEN_NI_INCLUDE)
# INCLUDE_DIRECTORIES(${OPEN_NI_INCLUDE})
# ELSE()
# # check if the envvar is set
# SET(OPEN_NI_INCLUDE $ENV{OPEN_NI_INCLUDE})
# IF(DEFINED OPEN_NI_INCLUDE)
# INCLUDE_DIRECTORIES(${OPEN_NI_INCLUDE})
# ELSE()
# MESSAGE("OPEN_NI_INCLUDE and OPEN_NI_LIB should be set, for example:")
# MESSAGE("> cmake -DOPEN_NI_INCLUDE=/usr/???/ -DOPEN_NI_LIB=/usr/???/ .")
# ENDIF()
# ENDIF()

# #INCLUDE_DIRECTORIES($ENV{OPEN_NI_INCLUDE})
# FIND_LIBRARY(OPEN_NI_LIBRARY NAMES OpenNI PATHS $ENV{OPEN_NI_LIB})
# SET(LIBS ${LIBS} ${OPEN_NI_LIBRARY})

# -----------------------------------------------------------------------------
# NITE

INCLUDE_DIRECTORIES(/usr/include/ni)
FIND_LIBRARY(NITE_LIBRARY NAMES XnVNite_1_5_2 PATHS ${XN_NITE_LIB})
SET(LIBS ${LIBS} ${NITE_LIBRARY})

#find_package (OpenNI REQUIRED)
#include_directories(${OPENNI_INCLUDE_DIRS})
#set(LIBS ${LIBS} ${OPENNI_LIBRARIES})

#GetEntityComponents()

MocFolder ()

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
#file (GLOB XML_FILES *.xml)
#file (GLOB UI_FILES ui/*.ui)
file (GLOB MOC_FILES OpenNIPlugin.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} ${UI_FILES} PARENT_SCOPE)

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})

use_core_modules(Framework)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS})

link_modules (Framework)

SetupCompileFlags()

final_target ()
