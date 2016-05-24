if(SHAIRPLAY_INCLUDE_DIR)
  # Already in cache, be silent
  set(SHAIRPLAY_FIND_QUIETLY TRUE)
endif(SHAIRPLAY_INCLUDE_DIR)

find_package(PkgConfig)
if (PKG_CONFIG_FOUND)
  pkg_check_modules(_SHAIRPLAY libshairplay)
endif (PKG_CONFIG_FOUND)

Find_Path(SHAIRPLAY_INCLUDE_DIR
  NAMES shairplay/raop.h
  PATHS /usr/include usr/local/include
  HINTS ${_SHAIRPLAY_INCLUDEDIR}
)

Find_Library(SHAIRPLAY_LIBRARY
  NAMES shairplay
  PATHS /usr/lib usr/local/lib
  HINTS ${_SHAIRPLAY_LIBDIR}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SHAIRPLAY DEFAULT_MSG SHAIRPLAY_LIBRARY SHAIRPLAY_INCLUDE_DIR)

IF(SHAIRPLAY_LIBRARY AND SHAIRPLAY_INCLUDE_DIR)
  plex_get_soname(SHAIRPLAY_SONAME ${SHAIRPLAY_LIBRARY})
ENDIF()
