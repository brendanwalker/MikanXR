# - Try to find GObject
# Once done this will define
#
#  GOBJECT_FOUND - system has GObject
#  GOBJECT_INCLUDE_DIR - the GObject include directory
#  GOBJECT_LIBRARIES - the libraries needed to use GObject
#  GOBJECT_DEFINITIONS - Compiler switches required for using GObject

# Copyright (c) 2006, Tim Beaulen <tbscope@gmail.com>
# Copyright (c) 2008 Helio Chissini de Castro, <helio@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


IF (GOBJECT_INCLUDE_DIR AND GOBJECT_LIBRARIES)
   # in cache already
   SET(GObject_FIND_QUIETLY TRUE)
ELSE (GOBJECT_INCLUDE_DIR AND GOBJECT_LIBRARIES)
   SET(GObject_FIND_QUIETLY FALSE)
ENDIF (GOBJECT_INCLUDE_DIR AND GOBJECT_LIBRARIES)

IF (NOT WIN32)
   FIND_PACKAGE(PkgConfig REQUIRED)
   # use pkg-config to get the directories and then use these values
   # in the FIND_PATH() and FIND_LIBRARY() calls
   PKG_CHECK_MODULES(PKG_GOBJECT2 REQUIRED gobject-2.0)
   SET(GOBJECT_DEFINITIONS ${PKG_GOBJECT2_CFLAGS})
ENDIF (NOT WIN32)

if (NOT GSTREAMER_ROOT)
    if (CMAKE_SIZEOF_VOID_P MATCHES "8")
		if (WIN32) 
			set(GSTREAMER_ROOT $ENV{GSTREAMER_1_0_ROOT_MINGW_X86_64})
		else ()
			set(GSTREAMER_ROOT $ENV{GSTREAMER_1_0_ROOT_X86_64})
		endif ()
    else ()
        set(GSTREAMER_ROOT $ENV{GSTREAMER_1_0_ROOT_X86})
    endif ()
endif ()

FIND_PATH(GOBJECT_INCLUDE_DIR gobject/gobject.h
   HINTS ${PKG_GOBJECT2_INCLUDE_DIRS} ${PKG_GOBJECT2_INCLUDEDIR}
   PATHS /usr/include/glib-2.0/ ${GSTREAMER_ROOT}/include
   PATH_SUFFIXES glib-2.0
   )

FIND_LIBRARY(_GObjectLibs NAMES gobject-2.0
   HINTS
   ${PKG_GOBJECT2_LIBRARY_DIRS}
   ${PKG_GOBJECT2_LIBDIR}
   ${GSTREAMER_ROOT}/lib
   )
FIND_LIBRARY(_GModuleLibs NAMES gmodule-2.0
   HINTS
   ${PKG_GOBJECT2_LIBRARY_DIRS}
   ${PKG_GOBJECT2_LIBDIR}
   ${GSTREAMER_ROOT}/lib
   )
FIND_LIBRARY(_GThreadLibs NAMES gthread-2.0
   HINTS
   ${PKG_GOBJECT2_LIBRARY_DIRS}
   ${PKG_GOBJECT2_LIBDIR}
   ${GSTREAMER_ROOT}/lib
   )
FIND_LIBRARY(_GLibs NAMES glib-2.0
   HINTS
   ${PKG_GOBJECT2_LIBRARY_DIRS}
   ${PKG_GOBJECT2_LIBDIR}
   ${GSTREAMER_ROOT}/lib
   )
FIND_LIBRARY(_GIOLibs NAMES gio-2.0
   HINTS
   ${PKG_GOBJECT2_LIBRARY_DIRS}
   ${PKG_GOBJECT2_LIBDIR}
   ${GSTREAMER_ROOT}/lib
   )

SET (GOBJECT_LIBRARIES ${_GObjectLibs} ${_GModuleLibs} ${_GThreadLibs} ${_GLibs} ${_GIOLibs})

MARK_AS_ADVANCED(GOBJECT_INCLUDE_DIR GOBJECT_LIBRARIES)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GObject DEFAULT_MSG GOBJECT_INCLUDE_DIR GOBJECT_LIBRARIES)
