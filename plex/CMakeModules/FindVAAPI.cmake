# -*- cmake -*-

if(VAAPI_INCLUDE_DIR)
  # Already in cache, be silent
  set(VAAPI_FIND_QUIETLY TRUE)
endif(VAAPI_INCLUDE_DIR)

find_package(PkgConfig)
if (PKG_CONFIG_FOUND)
  pkg_check_modules(_VAAPI libva)
  pkg_check_modules(_VAAPI_X11 libva-x11)
endif (PKG_CONFIG_FOUND)

Find_Path(VAAPI_MAIN_INCLUDE_DIR
  NAMES va/va.h
  PATHS /usr/include usr/local/include 
  HINTS ${_VAAPI_INCLUDEDIR}
)

Find_Path(VAAPI_X11_INCLUDE_DIR
  NAMES va/va_x11.h
  PATHS /usr/include usr/local/include 
  HINTS ${_VAAPI_INCLUDEDIR}
)

Find_Library(VAAPI_MAIN_LIBRARY
  NAMES va
  PATHS /usr/lib usr/local/lib
  HINTS ${_VAAPI_LIBDIR}
)

Find_Library(VAAPI_X11_LIBRARY
  NAMES va-x11
  PATHS /usr/lib usr/local/lib
  HINTS ${_VAAPI_LIBDIR}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VAAPI DEFAULT_MSG VAAPI_MAIN_LIBRARY VAAPI_MAIN_INCLUDE_DIR VAAPI_X11_LIBRARY VAAPI_X11_INCLUDE_DIR) 

IF(VAAPI_MAIN_LIBRARY AND VAAPI_MAIN_INCLUDE_DIR AND VAAPI_X11_INCLUDE_DIR AND VAAPI_X11_LIBRARY)
  set(VAAPI_LIBRARIES ${VAAPI_MAIN_LIBRARY} ${VAAPI_X11_LIBRARY})
  set(VAAPI_INCLUDE_DIRS ${VAAPI_MAIN_INCLUDE_DIR} ${VAAPI_X11_INCLUDE_DIR})
ENDIF()
