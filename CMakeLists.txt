cmake_minimum_required(VERSION 3.25)

include(CMakePackageConfigHelpers)

project(
    lumina
    DESCRIPTION "A C++ game framework focused on efficiency and transparency"
    VERSION 0.1.0
    LANGUAGES CXX
)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)

add_library(lumina INTERFACE)
add_library(lumina::lumina ALIAS lumina)

target_include_directories(lumina INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/source>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(lumina INTERFACE cxx_std_20)

target_link_libraries(lumina INTERFACE
    glfw
    Vulkan::Vulkan
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/luminaConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/luminaConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/luminaConfig.cmake"
    INSTALL_DESTINATION lib/cmake/lumina
)

install(
    DIRECTORY source/
    DESTINATION include
)

install(
    TARGETS lumina
    EXPORT luminaTargets
    INCLUDES DESTINATION include
)

install(
    EXPORT luminaTargets
    FILE luminaTargets.cmake
    NAMESPACE lumina::
    DESTINATION lib/cmake/lumina
)

install(
    FILES
    "${CMAKE_CURRENT_BINARY_DIR}/luminaConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/luminaConfigVersion.cmake"
    DESTINATION lib/cmake/lumina
)