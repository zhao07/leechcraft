cmake_minimum_required (VERSION 2.8)
project (leechcraft_blasq_spegnersi)
include (InitLCPlugin OPTIONAL)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package (QtKOAuth)

set (QT_USE_QTNETWORK TRUE)
set (QT_USE_QTXML TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	spegnersi.cpp
	flickrservice.cpp
	flickraccount.cpp
	)
#CreateTrs("blasq" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("blasq_spegnersi" "en;ru_RU" "${SRCS}" "${FORMS}" "")

add_library (leechcraft_blasq_spegnersi SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_blasq_spegnersi
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${QTKOAUTH_LIBRARIES}
	)
install (TARGETS leechcraft_blasq_spegnersi DESTINATION ${LC_PLUGINS_DEST})