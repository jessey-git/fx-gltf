# CMakeList.txt : CMake project for test, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

#############################################################################
# Catch library with the main function to speed up build
#############################################################################
set(THIRDPARTY_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)
set(FX_GLTF_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../include)

add_library(catch2_main OBJECT "src/unit.cpp" "src/utility.cpp")
target_compile_features(catch2_main PUBLIC cxx_std_17)
target_include_directories(catch2_main PRIVATE ${THIRDPARTY_INCLUDE_DIR})

if(MSVC)
    # Force to always compile with W4
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()
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
        COMPILE_OPTIONS "$<$<CXX_COMPILER_ID:MSVC>:/EHsc;$<$<CONFIG:Release>:/Od>>"
    )

    target_compile_definitions(${testcase} PRIVATE CATCH_CONFIG_FAST_COMPILE)
    target_compile_features(${testcase} PRIVATE cxx_std_17)
    target_include_directories(${testcase} PRIVATE ${THIRDPARTY_INCLUDE_DIR})
    target_include_directories(${testcase} PRIVATE ${FX_GLTF_INCLUDE_DIR})

    add_test(NAME "${testcase}"
      COMMAND ${testcase} ${CATCH_TEST_FILTER}
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties("${testcase}" PROPERTIES LABELS "default")

endforeach()