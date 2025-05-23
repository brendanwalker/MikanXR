# - Try to find GStreamer
# Once done this will define
#
#  GSTREAMER_FOUND - system has GStreamer
#  GSTREAMER_INCLUDE_DIR - the GStreamer include directory
#  GSTREAMER_LIBRARY - the main GStreamer library
#  GSTREAMER_PLUGIN_DIR - the GStreamer plugin directory
#
#  And for all the plugin libraries specified in the COMPONENTS
#  of find_package, this module will define:
#
#  GSTREAMER_<plugin_lib>_LIBRARY_FOUND - system has <plugin_lib>
#  GSTREAMER_<plugin_lib>_LIBRARY - the <plugin_lib> library
#  GSTREAMER_<plugin_lib>_INCLUDE_DIR - the <plugin_lib> include directory
#
# Copyright (c) 2010, Collabora Ltd.
#   @author George Kiagiadakis <george.kiagiadakis@collabora.co.uk>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (GSTREAMER_INCLUDE_DIR AND GSTREAMER_LIBRARY)
    set(GStreamer_FIND_QUIETLY TRUE)
else()
    set(GStreamer_FIND_QUIETLY FALSE)
endif()

set(GSTREAMER_ABI_VERSION "1.0")


# Find the main library
find_package(PkgConfig)

if (PKG_CONFIG_FOUND)
    pkg_check_modules(PKG_GSTREAMER gstreamer-${GSTREAMER_ABI_VERSION})
    execute_process(
				COMMAND ${PKG_CONFIG_EXECUTABLE} --variable pluginsdir gstreamer-${GSTREAMER_ABI_VERSION}
                OUTPUT_VARIABLE PKG_GSTREAMER_PLUGIN_DIR
				OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

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

find_library(GSTREAMER_LIBRARY
             NAMES gstreamer-${GSTREAMER_ABI_VERSION}
             HINTS ${PKG_GSTREAMER_LIBRARY_DIRS} ${PKG_GSTREAMER_LIBDIR} ${GSTREAMER_ROOT}/lib)

find_path(GSTREAMER_INCLUDE_DIR
          gst/gst.h
          HINTS ${PKG_GSTREAMER_INCLUDE_DIRS} ${PKG_GSTREAMER_INCLUDEDIR} ${GSTREAMER_ROOT}/include
          PATH_SUFFIXES gstreamer-${GSTREAMER_ABI_VERSION})

find_path(GSTREAMER_INCLUDE_DIR_ARCH
          gst/gstconfig.h
          HINTS ${PKG_GSTREAMER_INCLUDE_DIRS} ${PKG_GSTREAMER_INCLUDEDIR} ${GSTREAMER_ROOT}/lib/gstreamer-1.0/include
          PATH_SUFFIXES gstreamer-${GSTREAMER_ABI_VERSION})

if (GSTREAMER_INCLUDE_DIR_ARCH)
    set(GSTREAMER_INCLUDE_DIR ${GSTREAMER_INCLUDE_DIR} ${GSTREAMER_INCLUDE_DIR_ARCH})
endif ()

if (PKG_GSTREAMER_PLUGIN_DIR)
    set(_GSTREAMER_PLUGIN_DIR ${PKG_GSTREAMER_PLUGIN_DIR})
else()
    get_filename_component(_GSTREAMER_LIB_DIR ${GSTREAMER_LIBRARY} PATH)
    set(_GSTREAMER_PLUGIN_DIR ${_GSTREAMER_LIB_DIR}/gstreamer-${GSTREAMER_ABI_VERSION})
endif()

set(GSTREAMER_PLUGIN_DIR ${_GSTREAMER_PLUGIN_DIR}
    CACHE PATH "The path to the gstreamer plugins installation directory")

mark_as_advanced(GSTREAMER_LIBRARY GSTREAMER_INCLUDE_DIR GSTREAMER_PLUGIN_DIR)


# Find additional libraries
include(MacroFindGStreamerLibrary)

macro(_find_gst_component _name _header)
    find_gstreamer_library(${_name} ${_header} ${GSTREAMER_ABI_VERSION})
    set(_GSTREAMER_EXTRA_VARIABLES ${_GSTREAMER_EXTRA_VARIABLES}
                                    GSTREAMER_${_name}_LIBRARY GSTREAMER_${_name}_INCLUDE_DIR)
endmacro()

foreach(_component ${GStreamer_FIND_COMPONENTS})
    if (${_component} STREQUAL "base")
        _find_gst_component(BASE gstbasesink.h)
    elseif (${_component} STREQUAL "check")
        _find_gst_component(CHECK gstcheck.h)
    elseif (${_component} STREQUAL "controller")
        _find_gst_component(CONTROLLER gstcontroller.h)
    elseif (${_component} STREQUAL "dataprotocol")
        _find_gst_component(DATAPROTOCOL dataprotocol.h)
    elseif (${_component} STREQUAL "net")
        _find_gst_component(NET gstnet.h)
    else()
        message (AUTHOR_WARNING "FindGStreamerPluginsBase.cmake: Invalid component \"${_component}\" was specified")
    endif()
endforeach()


# Version check
if (GStreamer_FIND_VERSION)
    if (PKG_GSTREAMER_FOUND)
        if("${PKG_GSTREAMER_VERSION}" VERSION_LESS "${GStreamer_FIND_VERSION}")
            message(STATUS "Found GStreamer version ${PKG_GSTREAMER_VERSION}, but at least version ${GStreamer_FIND_VERSION} is required")
            set(GSTREAMER_VERSION_COMPATIBLE FALSE)
        else()
            set(GSTREAMER_VERSION_COMPATIBLE TRUE)
        endif()
    elseif(GSTREAMER_INCLUDE_DIR)
        include(CheckCXXSourceCompiles)

        set(CMAKE_REQUIRED_INCLUDES ${GSTREAMER_INCLUDE_DIR})
        string(REPLACE "." "," _comma_version ${GStreamer_FIND_VERSION})
        # Hack to invalidate the cached value
        set(GSTREAMER_VERSION_COMPATIBLE GSTREAMER_VERSION_COMPATIBLE)

        check_cxx_source_compiles("
#define G_BEGIN_DECLS
#define G_END_DECLS
#include <gst/gstversion.h>

#if GST_CHECK_VERSION(${_comma_version})
int main() { return 0; }
#else
# error \"GStreamer version incompatible\"
#endif
" GSTREAMER_VERSION_COMPATIBLE)

        if (NOT GSTREAMER_VERSION_COMPATIBLE)
            message(STATUS "GStreamer ${GStreamer_FIND_VERSION} is required, but the version found is older")
        endif()
    else()
        # We didn't find gstreamer at all
        set(GSTREAMER_VERSION_COMPATIBLE FALSE)
    endif()
else()
    # No version constrain was specified, thus we consider the version compatible
    set(GSTREAMER_VERSION_COMPATIBLE TRUE)
endif()


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GStreamer DEFAULT_MSG
                                  GSTREAMER_LIBRARY GSTREAMER_INCLUDE_DIR
                                  GSTREAMER_VERSION_COMPATIBLE ${_GSTREAMER_EXTRA_VARIABLES})
