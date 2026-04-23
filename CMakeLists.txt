cmake_minimum_required(VERSION 3.25)

include(CMakePackageConfigHelpers)

project(
    vesta
    DESCRIPTION "A C++ ECS library focused on efficiency"
    VERSION 0.1.0
    LANGUAGES CXX
)

add_library(vesta INTERFACE)
add_library(vesta::vesta ALIAS vesta)

target_include_directories(vesta INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/source>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(vesta INTERFACE cxx_std_20)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/vestaConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/vestaConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/vestaConfig.cmake"
    INSTALL_DESTINATION lib/cmake/vesta
)

install(
    DIRECTORY source/
    DESTINATION include
)

install(
    TARGETS vesta
    EXPORT vestaTargets
    INCLUDES DESTINATION include
)

install(
    EXPORT vestaTargets
    FILE vestaTargets.cmake
    NAMESPACE vesta::
    DESTINATION lib/cmake/vesta
)

install(
    FILES
    "${CMAKE_CURRENT_BINARY_DIR}/vestaConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/vestaConfigVersion.cmake"
    DESTINATION lib/cmake/vesta
)