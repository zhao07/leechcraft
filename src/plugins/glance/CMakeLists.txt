cmake_minimum_required (VERSION 2.8)
project (leechcraft_glance)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTNETWORK TRUE)
set (QT_USE_QTXML TRUE)

include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	core.cpp
	glance.cpp
	glanceitem.cpp
	glanceshower.cpp
	)
CreateTrs ("glance" "en;es;ru_RU;tt;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("glance" "en;ru_RU" "${SRCS}" "" "")

set (GLANCE_RESOURCES glanceresources.qrc)
QT4_ADD_RESOURCES (GLANCE_RCCS ${GLANCE_RESOURCES})

add_library (leechcraft_glance SHARED
	${GLANCE_RCCS}
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_glance
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_glance DESTINATION ${LC_PLUGINS_DEST})
