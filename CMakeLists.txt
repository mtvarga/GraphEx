# #############################################################################################################
# #                                     GRAPHEX MAIN CMAKE LISTS FILE                                         #
# #############################################################################################################
# #                                                                                                           #
# # Add your program executable/library targets below the custom function definitions where it is marked      #
# # It is not recommended to edit the rest of this file as you may break the CMake build procedure            #
# #                                                                                                           #
# #############################################################################################################


# Set GraphEx variables
set(GRAPHEX_TARGET_NAME "GraphEx")
set(GRAPHEX_TESTS_TARGET_NAME "GraphExTests")
set(GRAPHEX_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Source")
set(GRAPHEX_SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Shaders")
set(GRAPHEX_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Data")
set(GRAPHEX_DEPENDENCIES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies")
set(GRAPHEX_TESTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Tests")
set(FALCOR_DATA_OUTPUT_DIRECTORY "${FALCOR_OUTPUT_DIRECTORY}/data")

set_property(GLOBAL PROPERTY GRAPHEX_TARGET_SOURCE_GROUP_DATA "")


include(FetchContent)


function(graphex_copy_resource_directory directory destination_directory)
	file(GLOB_RECURSE files "${directory}/*.*")

	foreach (source ${files})
		if (NOT "${source}" MATCHES ".*CMakeLists.txt$")
			file(RELATIVE_PATH destination "${directory}" "${source}")
			set(destination "${destination_directory}/${destination}")

			add_custom_command(TARGET ${GRAPHEX_TARGET_NAME} PRE_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_if_different "${source}" "${destination}"
			)
		endif ()
	endforeach ()
endfunction()


function(target_source_group_flat target folder)
	set_target_properties(${target} PROPERTIES FOLDER ${folder})
	get_target_property(target_source_dir ${target} SOURCE_DIR)
	get_target_property(target_sources_ ${target} SOURCES)

	set(target_sources_fixed "")

	foreach(source ${target_sources_})
		if (NOT IS_ABSOLUTE ${source})
			list(APPEND target_sources_fixed "${target_source_dir}/${source}")
		else ()
			list(APPEND target_sources_fixed ${source})
		endif ()
	endforeach ()

	source_group(${target} FILES ${target_sources_fixed})
endfunction()


function (graphex_target_source_group target_name source_group_name)
	set_property(GLOBAL APPEND PROPERTY GRAPHEX_TARGET_SOURCE_GROUP_DATA "${target_name}")
	if (source_group_name STREQUAL "")
		set_property(GLOBAL APPEND_STRING PROPERTY GRAPHEX_TARGET_SOURCE_GROUP_DATA ";")
	else ()
		set_property(GLOBAL APPEND PROPERTY GRAPHEX_TARGET_SOURCE_GROUP_DATA "${source_group_name}")
	endif ()
endfunction()


function(graphex_process_target_source_group)
	get_property(target_source_group_data GLOBAL PROPERTY GRAPHEX_TARGET_SOURCE_GROUP_DATA)
	list(LENGTH target_source_group_data data_length)
	math(EXPR data_length "${data_length} - 1")
	foreach (i RANGE 0 ${data_length} 2)
		list(GET target_source_group_data ${i} target_name)
		math(EXPR i_plus_one "${i} + 1")
		list(GET target_source_group_data ${i_plus_one} source_group_name)

		get_target_property(target_source_dir ${target_name} SOURCE_DIR)
		get_target_property(target_sources_ ${target_name} SOURCES)

		set(target_sources_fixed "")

		foreach(source ${target_sources_})
			if (NOT IS_ABSOLUTE ${source})
				list(APPEND target_sources_fixed "${target_source_dir}/${source}")
			else ()
				list(APPEND target_sources_fixed ${source})
			endif ()
		endforeach ()

		if (NOT source_group_name STREQUAL "")
			set_target_properties(${target_name} PROPERTIES FOLDER ${source_group_name})
		endif ()

		source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${target_sources_fixed} PREFIX "")
	endforeach ()
endfunction()


function(graphex_add_library target_name library_type)
	add_library(${target_name} ${library_type})
	set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${FALCOR_RUNTIME_OUTPUT_DIRECTORY})
	set_target_properties(${target_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${FALCOR_LIBRARY_OUTPUT_DIRECTORY})
	set_target_properties(${target_name} PROPERTIES VS_DEBUGGER_ENVIRONMENT "FALCOR_DEVMODE=1")

	if (NOT ${target_name} STREQUAL ${GRAPHEX_TARGET_NAME})
		target_link_libraries(${target_name} PRIVATE ${GRAPHEX_TARGET_NAME})
	else ()
		target_link_libraries(${target_name} PUBLIC Falcor)
	endif ()
endfunction()


function(graphex_add_executable target_name)
	add_falcor_executable(${target_name})
	target_link_libraries(${target_name} PRIVATE ${GRAPHEX_TARGET_NAME})
endfunction()


function(graphex_add_test_suite target)
	graphex_add_executable(${target})
	target_link_libraries(${target} PRIVATE gtest_main)
	add_test(NAME ${target} COMMAND ${target})
endfunction()


function(get_shader_files_from_directories out_var)
	set(all_shader_files "")
	foreach (dir ${ARGN})
		file(GLOB_RECURSE shader_files
			"${dir}/*.slang"
			"${dir}/*.slangh"
			"${dir}/*.hlsl"
			"${dir}/*.hlsli"
		)
		list(APPEND all_shader_files ${shader_files})
	endforeach ()
	set(${out_var} ${all_shader_files} PARENT_SCOPE)
endfunction()


graphex_add_library(${GRAPHEX_TARGET_NAME} STATIC)
graphex_add_test_suite(${GRAPHEX_TESTS_TARGET_NAME})



# #############################################################################################################
# #                                ---- Add your program libraries here ----                                  #
# #############################################################################################################

# graphex_add_library("MyStaticLib" STATIC)
# graphex_add_library("MyDynamicLib" DYNAMIC)


# #############################################################################################################
# #                               ---- Add your program executables here ----                                 #
# #############################################################################################################

# graphex_add_executable("MyExecutable")

# #############################################################################################################
# #                               ---- Add your program test suites here ----                                 #
# #############################################################################################################

# graphex_add_test_suite("MyTestSuite")



add_subdirectory(${GRAPHEX_SOURCE_DIR})
add_subdirectory(${GRAPHEX_DATA_DIR})
add_subdirectory(${GRAPHEX_DEPENDENCIES_DIR})
add_subdirectory(${GRAPHEX_SHADER_SOURCE_DIR})
add_subdirectory(${GRAPHEX_TESTS_DIR})


# Copy shader files and data files to Falcor's shader and data folders
graphex_copy_resource_directory(${GRAPHEX_SHADER_SOURCE_DIR} ${FALCOR_SHADER_OUTPUT_DIRECTORY})
graphex_copy_resource_directory(${GRAPHEX_DATA_DIR} ${FALCOR_DATA_OUTPUT_DIRECTORY})

# Add Source directory to include directories
target_include_directories(${GRAPHEX_TARGET_NAME} PUBLIC ${GRAPHEX_SOURCE_DIR})

FetchContent_Declare(
	googletest
	# Specify the commit you depend on and update it regularly.
	URL https://github.com/google/googletest/archive/refs/tags/v1.16.0.zip
	DOWNLOAD_EXTRACT_TIMESTAMP true
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

target_source_group_flat(gmock "Libraries")
target_source_group_flat(gmock_main "Libraries")
target_source_group_flat(gtest "Libraries")
target_source_group_flat(gtest_main "Libraries")

graphex_process_target_source_group()


install(TARGETS ${GRAPHEX_TARGET_NAME}
	LIBRARY DESTINATION ${FALCOR_INSTALL_LIB_DIR}
)
