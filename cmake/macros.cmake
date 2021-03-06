macro(engine_install TARGET FILE DESTINATION INSTALL_DATA)
	set(INSTALL_DATA_DIR "${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}-${TARGET}")
	if (INSTALL_DATA)
		install(FILES ${DATA_DIR}/${FILE} DESTINATION ${INSTALL_DATA_DIR}/${DESTINATION} COMPONENT ${TARGET})
	endif()
	get_filename_component(filename ${FILE} NAME)
	configure_file(${DATA_DIR}/${FILE} ${CMAKE_BINARY_DIR}/${TARGET}/${DESTINATION}/${filename} COPYONLY)
endmacro()

#
# set up the binary for the application. This will also set up platform specific stuff for you
#
# Example: engine_add_executable(TARGET SomeTargetName SRCS Source.cpp Main.cpp WINDOWED)
#
# TARGET:    the target name (binary name)
# SRCS:      the source files for this target
# WINDOWED:  this is needed to indicate whether the application should e.g. spawn a console on windows
# NOINSTALL: means that the binary and data files are not put into the final installation folder
#            this can e.g. be useful for stuff like code generators that are only needed during build
#            time.
#
macro(engine_add_executable)
	set(_OPTIONS_ARGS WINDOWED NOINSTALL)
	set(_ONE_VALUE_ARGS TARGET)
	set(_MULTI_VALUE_ARGS SRCS LUA_SRCS FILES)

	cmake_parse_arguments(_EXE "${_OPTIONS_ARGS}" "${_ONE_VALUE_ARGS}" "${_MULTI_VALUE_ARGS}" ${ARGN} )

	# e.g. used in desktop files
	set(COMMANDLINE "${CMAKE_PROJECT_NAME}-${_EXE_TARGET}")
	set(CATEGORIES "Game")
	set(DESCRIPTION "")
	set(NAME ${_EXE_TARGET})
	set(APPICON "${_EXE_TARGET}-icon")
	set(ICON "${APPICON}.png")

	set(${_EXE_TARGET}_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "${_EXE_TARGET} source directory")
	if (_EXE_WINDOWED)
		generate_unity_sources(WINDOWED EXECUTABLE TARGET ${_EXE_TARGET} SRCS ${_EXE_SRCS})
	else()
		generate_unity_sources(EXECUTABLE TARGET ${_EXE_TARGET} SRCS ${_EXE_SRCS})
	endif()
	set_target_properties(${_EXE_TARGET} PROPERTIES OUTPUT_NAME "${CMAKE_PROJECT_NAME}-${_EXE_TARGET}")
	set_target_properties(${_EXE_TARGET} PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${_EXE_TARGET}"
		LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${_EXE_TARGET}"
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${_EXE_TARGET}"
	)
	foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
		string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
		set_target_properties(${_EXE_TARGET} PROPERTIES
			ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_BINARY_DIR}/${_EXE_TARGET}"
			LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_BINARY_DIR}/${_EXE_TARGET}"
			RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_BINARY_DIR}/${_EXE_TARGET}"
		)
	endforeach()

	if (_EXE_LUA_SRCS)
		check_lua_files(${_EXE_TARGET} ${_EXE_LUA_SRCS})
	endif()

	set(INSTALL_DATA_DIR "${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}-${_EXE_TARGET}")
	set(INSTALL_ICON_DIR "${CMAKE_INSTALL_DATADIR}/icons")
	set(INSTALL_APPLICATION_DIR "${CMAKE_INSTALL_DATADIR}/applications")

	if (SANITIZER_THREADS AND NOT ${_EXE_TARGET} STREQUAL "databasetool" AND NOT ${_EXE_TARGET} STREQUAL "shadertool" AND NOT ${_EXE_TARGET} STREQUAL "uitool")
		set_target_properties(${_EXE_TARGET} PROPERTIES COMPILE_FLAGS "${SANITIZE_THREAD_FLAG}")
		set_target_properties(${_EXE_TARGET} PROPERTIES LINK_FLAGS "${SANITIZE_THREAD_FLAG}")
	endif()

	if (_EXE_NOINSTALL)
		set(INSTALL_DATA False)
	else()
		set(INSTALL_DATA True)
	endif()

	if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
		if (_EXE_WINDOWED)
			configure_file(${ROOT_DIR}/contrib/installer/linux/desktop.in ${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}-${_EXE_TARGET}.desktop)
			if (DESKTOP_FILE_VALIDATE_EXECUTABLE)
				add_custom_command(TARGET ${_EXE_TARGET} POST_BUILD
					COMMAND ${DESKTOP_FILE_VALIDATE_EXECUTABLE} ${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}-${_EXE_TARGET}.desktop
					COMMENT "Validate ${CMAKE_PROJECT_NAME}-${_EXE_TARGET}.desktop"
				)
			endif()
			if (INSTALL_DATA)
				install(FILES ${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}-${_EXE_TARGET}.desktop DESTINATION ${INSTALL_APPLICATION_DIR})
			endif()
		endif()
		if (EXISTS ${ROOT_DIR}/contrib/installer/linux/${_EXE_TARGET}.service.in)
			# TODO systemd-analyze --user  verify build/Debug/src/server/vengi-server.service
			configure_file(${ROOT_DIR}/contrib/installer/linux/${_EXE_TARGET}.service.in ${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}-${_EXE_TARGET}.service)
			if (INSTALL_DATA)
				install(FILES ${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}-${_EXE_TARGET}.service DESTINATION lib/systemd/user)
			endif()
		endif()
	endif()

	set_property(GLOBAL PROPERTY ${_EXE_TARGET}_EXECUTABLE True)
	set_property(GLOBAL PROPERTY ${_EXE_TARGET}_INSTALL ${INSTALL_DATA})
	set_property(GLOBAL PROPERTY ${_EXE_TARGET}_FILES "${_EXE_FILES}")

	foreach (luasrc ${_EXE_LUA_SRCS})
		get_filename_component(luasrcdir ${luasrc} DIRECTORY)
		if (INSTALL_DATA)
			install(FILES lua/${luasrc} DESTINATION ${INSTALL_DATA_DIR}/${luasrcdir} COMPONENT ${_EXE_TARGET})
		endif()
		get_filename_component(filename ${luasrc} NAME)
		get_filename_component(datafiledir ${luasrc} DIRECTORY)
		configure_file(lua/${luasrc} ${CMAKE_BINARY_DIR}/${_EXE_TARGET}/${datafiledir}/${filename} COPYONLY)
	endforeach()

	if (EXISTS ${ROOT_DIR}/contrib/${ICON})
		if (INSTALL_DATA)
			install(FILES ${ROOT_DIR}/contrib/${ICON} DESTINATION ${INSTALL_ICON_DIR} COMPONENT ${_EXE_TARGET})
		endif()
	endif()
	if (INSTALL_DATA)
		install(TARGETS ${_EXE_TARGET} DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT ${_EXE_TARGET})
	endif()
	add_custom_target(${_EXE_TARGET}-run COMMAND $<TARGET_FILE:${_EXE_TARGET}> DEPENDS ${_EXE_TARGET} WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/${_EXE_TARGET}")
	engine_add_debuggger(${_EXE_TARGET})
	engine_add_valgrind(${_EXE_TARGET})
	engine_add_perf(${_EXE_TARGET})
	if (_EXE_WINDOWED)
		engina_add_vogl(${_EXE_TARGET})
	endif()
endmacro()

macro(engine_add_module)
	set(_OPTIONS_ARGS)
	set(_ONE_VALUE_ARGS TARGET)
	set(_MULTI_VALUE_ARGS SRCS FILES DEPENDENCIES)

	cmake_parse_arguments(_LIB "${_OPTIONS_ARGS}" "${_ONE_VALUE_ARGS}" "${_MULTI_VALUE_ARGS}" ${ARGN})

	set(${_LIB_TARGET}_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "${_LIB_TARGET} module source directory")
	generate_unity_sources(LIBRARY TARGET ${_LIB_TARGET} SRCS ${_LIB_SRCS})

	set_target_properties(${_LIB_TARGET} PROPERTIES FOLDER ${_LIB_TARGET})
	if (_LIB_DEPENDENCIES)
		target_link_libraries(${_LIB_TARGET} ${_LIB_DEPENDENCIES})
		foreach (dep ${_LIB_DEPENDENCIES})
			get_property(DEP_FILES GLOBAL PROPERTY ${dep}_FILES)
			list(APPEND _LIB_FILES ${DEP_FILES})
		endforeach()
	endif()
	set_property(GLOBAL PROPERTY ${_LIB_TARGET}_FILES ${_LIB_FILES})
	set_property(GLOBAL PROPERTY ${_LIB_TARGET}_DEPENDENCIES ${_LIB_DEPENDENCIES})
endmacro()

macro(engine_target_link_libraries)
	set(_OPTIONS_ARGS)
	set(_ONE_VALUE_ARGS TARGET)
	set(_MULTI_VALUE_ARGS DEPENDENCIES)

	cmake_parse_arguments(_LIBS "${_OPTIONS_ARGS}" "${_ONE_VALUE_ARGS}" "${_MULTI_VALUE_ARGS}" ${ARGN})

	set_property(GLOBAL PROPERTY ${_LIBS_TARGET}_DEPENDENCIES ${_LIBS_DEPENDENCIES})
	target_link_libraries(${_LIBS_TARGET} ${_LIBS_DEPENDENCIES})

	get_property(EXECUTABLE GLOBAL PROPERTY ${_LIBS_TARGET}_EXECUTABLE)
	if (EXECUTABLE)
		get_property(INSTALL_DATA GLOBAL PROPERTY ${_LIBS_TARGET}_INSTALL)
		get_property(INSTALL_FILES GLOBAL PROPERTY ${_LIBS_TARGET}_FILES)
		foreach (dep ${_LIBS_DEPENDENCIES})
			get_property(FILES GLOBAL PROPERTY ${dep}_FILES)
			list(APPEND INSTALL_FILES ${FILES})
		endforeach()

		if (INSTALL_FILES)
			list(REMOVE_DUPLICATES INSTALL_FILES)
			list(REVERSE INSTALL_FILES)
			foreach (datafile ${INSTALL_FILES})
				string(REGEX REPLACE "^[^/]+" "" target_datafile "${datafile}")
				string(LENGTH ${target_datafile} target_datafile_length)
				string(SUBSTRING ${target_datafile} 1 ${target_datafile_length} target_datafile)
				get_filename_component(datafiledir ${target_datafile} DIRECTORY)
				engine_install(${_LIBS_TARGET} "${datafile}" "${datafiledir}" ${INSTALL_DATA})
			endforeach()
		endif()
	endif()
endmacro()
