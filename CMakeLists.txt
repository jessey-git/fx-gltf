cmake_minimum_required (VERSION 3.8)

##
## Config
##
set(FX_GLTF_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

##
## Testing
##
include(CTest)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()
