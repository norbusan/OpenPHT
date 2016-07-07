if(DRM_INCLUDE_DIR)
  # Already in cache, be silent
  set(DRM_FIND_QUIETLY TRUE)
endif(DRM_INCLUDE_DIR)

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(_DRM libdrm)
endif(PKG_CONFIG_FOUND)

find_path(DRM_INCLUDE_DIR
  NAMES drm.h
  PATHS /usr/include usr/local/include
  HINTS ${_DRM_INCLUDE_DIRS}
)

find_library(DRM_LIBRARY
  NAMES drm
  PATHS /usr/lib usr/local/lib
  HINTS ${_DRM_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DRM DEFAULT_MSG DRM_INCLUDE_DIR DRM_LIBRARY)

mark_as_advanced(DRM_INCLUDE_DIR DRM_LIBRARY)
