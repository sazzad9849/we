# - Try to find FFmpeg
# Using Pkg-config if available for path
#
#  FFMPEG_FOUND        - all required ffmpeg components found on system
#  FFMPEG_INCLUDE_DIRS  - combined include directories
#  FFMPEG_LIBRARIES    - combined libraries to link

set(REQUIRED_AVCODEC_VERSION 0.8)
set(REQUIRED_AVCODEC_API_VERSION 53.25.0)

include(FindPkgConfig)

if (PKG_CONFIG_FOUND)
	pkg_check_modules(AVCODEC libavcodec)
	pkg_check_modules(AVUTIL libavutil)
	pkg_check_modules(SWRESAMPLE libswresample)
endif(PKG_CONFIG_FOUND)

# avcodec
find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h PATHS ${AVCODEC_INCLUDE_DIRS})
find_library(AVCODEC_LIBRARY avcodec PATHS $ {AVCODEC_LIBRARY_DIRS})

# avutil
find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h PATHS ${AVUTIL_INCLUDE_DIRS})
find_library(AVUTIL_LIBRARY avutil PATHS ${AVUTIL_LIBRARY_DIRS})

# swresample
find_path(SWRESAMPLE_INCLUDE_DIR libswresample/swresample.h PATHS ${SWRESAMPLE_INCLUDE_DIRS})
find_library(SWRESAMPLE_LIBRARY swresample PATHS ${SWRESAMPLE_LIBRARY_DIRS})


if (AVCODEC_INCLUDE_DIR AND AVCODEC_LIBRARY AND SWRESAMPLE_LIBRARY)
	set(AVCODEC_FOUND TRUE)
endif(AVCODEC_INCLUDE_DIR AND AVCODEC_LIBRARY AND SWRESAMPLE_LIBRARY)

if (AVUTIL_INCLUDE_DIR AND AVUTIL_LIBRARY)
	set(AVUTIL_FOUND TRUE)
endif(AVUTIL_INCLUDE_DIR AND AVUTIL_LIBRARY)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FFmpeg DEFAULT_MSG AVUTIL_FOUND AVCODEC_FOUND SWRESAMPLE_FOUND)

if (AVCODEC_VERSION)
	if (${AVCODEC_VERSION} VERSION_LESS ${REQUIRED_AVCODEC_API_VERSION})
		message(FATAL_ERROR
			    "libavcodec version >= ${REQUIRED_AVCODEC_VERSION} (API >= ${REQUIRED_AVCODEC_API_VERSION}) is required")
	endif()
else(AVCODEC_VERSION)
	message("Note: To build libavcodec version >= ${REQUIRED_AVCODEC_VERSION} (API >= ${REQUIRED_AVCODEC_API_VERSION}) is required")
endif(AVCODEC_VERSION)

if (FFMPEG_FOUND)
	set(FFMPEG_INCLUDE_DIRS ${AVCODEC_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIR} ${SWRESAMPLE_INCLUDE_DIR})
	set(FFMPEG_LIBRARIES ${AVCODEC_LIBRARY} ${AVUTIL_LIBRARY} ${SWRESAMPLE_LIBRARY})
endif(FFMPEG_FOUND)

mark_as_advanced(FFMPEG_INCLUDE_DIRS FFMPEG_LIBRARIES)
