# - Try to find QCA2 (Qt Cryptography Architecture 2)
# Once done this will define
#
#  QCA2_FOUND - system has QCA2
#  QCA2_INCLUDE_DIR - the QCA2 include directory
#  QCA2_LIBRARIES - the libraries needed to use QCA2
#  QCA2_DEFINITIONS - Compiler switches required for using QCA2
#
# use pkg-config to get the directories and then use these values
# in the find_path() and find_library() calls

# Copyright (c) 2006, Michael Larouche, <michael.larouche@kdemail.net>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(FindLibraryWithDebug)

if (QCA2_INCLUDE_DIR AND QCA2_LIBRARIES)

  # in cache already
	set(QCA2_FOUND TRUE)

else ()

	if (NOT WIN32)
		find_package(PkgConfig)
		pkg_check_modules(PC_QCA2 QUIET qca2)
		set(QCA2_DEFINITIONS ${PC_QCA2_CFLAGS_OTHER})
	endif ()

	find_library_with_debug(QCA2_LIBRARIES
					WIN32_DEBUG_POSTFIX d
					NAMES qca 
					HINTS ${PC_QCA2_LIBDIR} ${PC_QCA2_LIBRARY_DIRS} ${QCA2_DIR}/lib
					)

	find_path(QCA2_INCLUDE_DIR QtCrypto
				HINTS ${PC_QCA2_INCLUDEDIR} ${PC_QCA2_INCLUDE_DIRS} ${QCA2_DIR}/include
				PATH_SUFFIXES QtCrypto)

	
	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(QCA2  DEFAULT_MSG  QCA2_LIBRARIES QCA2_INCLUDE_DIR)
	message(STATUS "Found the QCA libraries at ${QCA2_LIBRARIES} Includes at ${QCA2_INCLUDE_DIR}")

	mark_as_advanced(QCA2_INCLUDE_DIR QCA2_LIBRARIES)
endif ()
