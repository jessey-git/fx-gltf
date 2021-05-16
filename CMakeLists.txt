cmake_minimum_required(VERSION 3.8)
project(fx-gltf VERSION 1.0.2)

if (POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()

if (POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()

option(FX_GLTF_BUILD_TESTS "If the tests should be built" ON)
option(FX_GLTF_INSTALL "If the library should be installed" ON)
option(FX_GLTF_USE_INSTALLED_DEPS "If installed or repo local dependencies should be used" OFF)

# Force use of installed deps if installing
if(FX_GLTF_INSTALL)
	set(FX_GLTF_USE_INSTALLED_DEPS ON)
endif()

##
## Config
##
set(FX_GLTF_LIB_TARGET_NAME ${PROJECT_NAME})
set(FX_GLTF_TARGETS_EXPORT_NAME ${PROJECT_NAME}Targets)
set(FX_GLTF_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/)
set(FX_GLTF_INSTALL_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include)
set(FX_GLTF_INSTALL_CONFIG_DIR ${CMAKE_INSTALL_PREFIX}/lib/cmake/${PROJECT_NAME})
set(FX_GLTF_LICENSE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)
set(FX_GLTF_TARGETS_FILE ${PROJECT_NAME}Targets.cmake)
set(FX_GLTF_CONFIG_FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake)
set(FX_GLTF_CONFIG_VERSION_FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake)
set(FX_GLTF_CONFIG_TEMPLATE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in)

##
## Library target
##
add_library(${FX_GLTF_LIB_TARGET_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${FX_GLTF_LIB_TARGET_NAME} ALIAS ${FX_GLTF_LIB_TARGET_NAME})
target_include_directories(${FX_GLTF_LIB_TARGET_NAME}
    INTERFACE
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${FX_GLTF_INCLUDE_DIR}>
)
target_compile_features(${FX_GLTF_LIB_TARGET_NAME} INTERFACE cxx_std_14)

## Dependencies
if(FX_GLTF_USE_INSTALLED_DEPS)
	find_package(nlohmann_json 3.8.0 REQUIRED)
	target_link_libraries(${FX_GLTF_LIB_TARGET_NAME} INTERFACE nlohmann_json::nlohmann_json)
endif()

##
## Testing
##
include(CTest)

if(FX_GLTF_BUILD_TESTS AND BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()

##
## Install
##
if(FX_GLTF_INSTALL)
	include(CMakePackageConfigHelpers)
	write_basic_package_version_file(
		${FX_GLTF_CONFIG_VERSION_FILE}
		VERSION ${PROJECT_VERSION}
		COMPATIBILITY SameMajorVersion
	)
	configure_package_config_file(
		${FX_GLTF_CONFIG_TEMPLATE_FILE}
		${FX_GLTF_CONFIG_FILE}
		INSTALL_DESTINATION ${FX_GLTF_INSTALL_CONFIG_DIR}
	)

	install(
		TARGETS ${FX_GLTF_LIB_TARGET_NAME}
		EXPORT ${FX_GLTF_TARGETS_EXPORT_NAME}
	)
	install(
		DIRECTORY ${FX_GLTF_INCLUDE_DIR}
		DESTINATION ${FX_GLTF_INSTALL_INCLUDE_DIR}
	)
	install(
		EXPORT ${FX_GLTF_TARGETS_EXPORT_NAME}
		FILE ${FX_GLTF_TARGETS_FILE}
		NAMESPACE ${PROJECT_NAME}::
		DESTINATION ${FX_GLTF_INSTALL_CONFIG_DIR}
	)
	install(
		FILES
			${FX_GLTF_CONFIG_FILE}
			${FX_GLTF_CONFIG_VERSION_FILE}
		DESTINATION ${FX_GLTF_INSTALL_CONFIG_DIR}
	)
	install(
		FILES ${FX_GLTF_LICENSE_FILE}
		DESTINATION ${CMAKE_INSTALL_PREFIX}
	)

	##
	## Export
	##
	export(
		EXPORT ${FX_GLTF_TARGETS_EXPORT_NAME}
		FILE ${CMAKE_CURRENT_BINARY_DIR}/${FX_GLTF_TARGETS_FILE}
		NAMESPACE ${PROJECT_NAME}::
	)
	export(PACKAGE ${PROJECT_NAME})
endif()
