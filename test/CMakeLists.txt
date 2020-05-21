set(THIRDPARTY_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)

set(TEST_LanguageStandard "17" CACHE STRING "14 or 17")

#############################################################################
# Catch library with the main function to speed up build
#############################################################################
add_library(catch2_main OBJECT "src/unit.cpp" "src/utility.cpp")
target_include_directories(catch2_main PRIVATE "src")

if(FX_GLTF_USE_INSTALLED_DEPS)
	find_package(Catch2 2.6.0 REQUIRED)

	target_link_libraries(catch2_main PRIVATE nlohmann_json::nlohmann_json)
	target_link_libraries(catch2_main PRIVATE Catch2::Catch2)
else()
	target_include_directories(catch2_main PRIVATE ${THIRDPARTY_INCLUDE_DIR})
endif()

if(TEST_LanguageStandard STREQUAL "17")
    target_compile_features(catch2_main PUBLIC cxx_std_17)
elseif(TEST_LanguageStandard STREQUAL "14")
    target_compile_features(catch2_main PUBLIC cxx_std_14)
endif()

if(MSVC)
    # Force to always compile with W4
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive-")
endif()

#############################################################################
# One executable for each unit test file
#############################################################################

file(GLOB files "src/unit-*.cpp")
foreach(file ${files})
    get_filename_component(file_basename ${file} NAME_WE)
    string(REGEX REPLACE "unit-([^$]+)" "test-\\1" testcase ${file_basename})

    add_executable(${testcase} $<TARGET_OBJECTS:catch2_main> ${file})
    set_target_properties(${testcase} PROPERTIES
        COMPILE_DEFINITIONS "$<$<CXX_COMPILER_ID:MSVC>:_SCL_SECURE_NO_WARNINGS>"
        COMPILE_OPTIONS "$<$<CXX_COMPILER_ID:MSVC>:/EHsc>"
    )

    if(TEST_LanguageStandard STREQUAL "17")
        target_compile_features(${testcase} PRIVATE cxx_std_17)
    elseif(TEST_LanguageStandard STREQUAL "14")
        target_compile_features(${testcase} PRIVATE cxx_std_14)
    endif()

    target_compile_definitions(${testcase} PRIVATE CATCH_CONFIG_FAST_COMPILE)
    target_link_libraries(${testcase} PRIVATE ${FX_GLTF_LIB_TARGET_NAME})
	if(FX_GLTF_USE_INSTALLED_DEPS)
		target_link_libraries(${testcase} PRIVATE nlohmann_json::nlohmann_json)
		target_link_libraries(${testcase} PRIVATE Catch2::Catch2)
	else()
		target_include_directories(${testcase} PRIVATE ${THIRDPARTY_INCLUDE_DIR})
	endif()

    if(CMAKE_CXX_COMPILER_ID STREQUAL GNU OR CMAKE_CXX_COMPILER_ID STREQUAL Clang)
        target_link_libraries(${testcase} PRIVATE stdc++fs)
    endif()

    add_test(NAME "${testcase}"
      COMMAND ${testcase} ${CATCH_TEST_FILTER}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_tests_properties("${testcase}" PROPERTIES LABELS "default")

endforeach()