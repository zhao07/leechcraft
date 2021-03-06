cmake_minimum_required (VERSION 2.8)
project (leechcraft_xtazy)
include (InitLCPlugin OPTIONAL)

if (NOT WIN32)
	set (QT_USE_QTDBUS TRUE)
endif ()
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	xtazy.cpp
	xmlsettingsmanager.cpp
	tunesourcebase.cpp
	filesource.cpp
	lcsource.cpp
	)

if (QT_QTDBUS_FOUND)
	add_definitions (-DHAVE_DBUS)

	set (SRCS
		${SRCS}
		mprissource.cpp
		)
endif ()

CreateTrs("xtazy" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("xtazy" "en;ru_RU" "${SRCS}" "${FORMS}" "xtazysettings.xml")

add_library (leechcraft_xtazy SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_xtazy
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_xtazy DESTINATION ${LC_PLUGINS_DEST})
install (FILES xtazysettings.xml DESTINATION ${LC_SETTINGS_DEST})
