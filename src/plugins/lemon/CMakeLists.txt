cmake_minimum_required (VERSION 2.8)
project (leechcraft_lemon)
include (InitLCPlugin OPTIONAL)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package (Libnl REQUIRED)
find_package (Qwt REQUIRED)

set (QT_USE_QTNETWORK TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LIBNL_INCLUDE_DIR}
	${QWT_INCLUDE_DIRS}
	)
set (SRCS
	lemon.cpp
	core.cpp
	trafficmanager.cpp
	trafficdialog.cpp
	platformbackend.cpp
	linuxplatformbackend.cpp
	)
set (FORMS
	trafficdialog.ui
	)
QT4_WRAP_UI (UIS_H ${FORMS})

CreateTrs("lemon" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("lemon" "en;ru_RU" "${SRCS}" "" "")

add_library (leechcraft_lemon SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	)
target_link_libraries (leechcraft_lemon
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${LIBNL_LIBRARIES}
	${QWT_LIBRARIES}
	)
install (TARGETS leechcraft_lemon DESTINATION ${LC_PLUGINS_DEST})
install (DIRECTORY share/qml/lemon DESTINATION ${LC_QML_DEST})
