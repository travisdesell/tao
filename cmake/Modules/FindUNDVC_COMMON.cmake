# - Try to find LibXml2
# Once done this will define
#  UNDVC_COMMON_FOUND - System has LibXml2
#  UNDVC_COMMON_INCLUDE_DIRS - The LibXml2 include directories
#  UNDVC_COMMON_LIBRARIES - The libraries needed to use LibXml2
#  UNDVC_COMMON_DEFINITIONS - Compiler switches required for using LibXml2

find_package(PkgConfig)
pkg_check_modules(PC_LIBXML QUIET undvc_common)

set(UNDVC_COMMON_DEFINITIONS ${UNDVC_COMMON_CFLAGS_OTHER})

find_path(UNDVC_COMMON_INCLUDE_DIR vector_io.hxx arguments.hxx
          HINTS ${UNDVC_COMMON_INCLUDEDIR} ${UNDVC_COMMON_INCLUDE_DIRS}
          PATH_SUFFIXES undvc_common)

find_library(UNDVC_COMMON_LIBRARY NAMES undvc_common 
             HINTS ${UNDVC_COMMON_LIBDIR} ${UNDVC_COMMON_LIBRARY_DIRS}
             PATH_SUFFIXES undvc_common/build/)

set(UNDVC_COMMON_LIBRARIES ${UNDVC_COMMON_LIBRARY} )
set(UNDVC_COMMON_INCLUDE_DIRS ${UNDVC_COMMON_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set UNDVC_COMMON_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(undvc_common DEFAULT_MSG
                                  UNDVC_COMMON_LIBRARY UNDVC_COMMON_INCLUDE_DIR)

mark_as_advanced(UNDVC_COMMON_INCLUDE_DIR UNDVC_COMMON_LIBRARY)
