cmake_minimum_required(VERSION 3.25)

include(CMakePackageConfigHelpers)

project(
    eecs
    DESCRIPTION "A C++ ECS library focused on efficiency"
    VERSION 0.1.0
    LANGUAGES CXX
)

add_library(eecs INTERFACE)
add_library(eecs::eecs ALIAS eecs)

target_include_directories(eecs INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/source>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(eecs INTERFACE cxx_std_20)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/eecsConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/eecsConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/eecsConfig.cmake"
    INSTALL_DESTINATION lib/cmake/eecs
)

install(
    DIRECTORY source/
    DESTINATION include
)

install(
    TARGETS eecs
    EXPORT eecsTargets
    INCLUDES DESTINATION include
)

install(
    EXPORT eecsTargets
    FILE eecsTargets.cmake
    NAMESPACE eecs::
    DESTINATION lib/cmake/eecs
)

install(
    FILES
    "${CMAKE_CURRENT_BINARY_DIR}/eecsConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/eecsConfigVersion.cmake"
    DESTINATION lib/cmake/eecs
)