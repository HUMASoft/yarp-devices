# Copyright: Universidad Carlos III de Madrid (C) 2019
# Released under the terms of the GNU LGPL v2.1

cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(HUMASOFT_YARP_DEVICES)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add suffix for debug libraries.
if(MSVC)
    message(STATUS "Running on windows")
    set(CMAKE_DEBUG_POSTFIX "d")
endif()

# Let the user specify a configuration (only single-config generators).
if(NOT CMAKE_CONFIGURATION_TYPES)
    # Possible values.
    set(_configurations Debug Release MinSizeRel RelWithDebInfo)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${_configurations})

    foreach(_conf ${_configurations})
        set(_conf_string "${_conf_string} ${_conf}")
    endforeach()

    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY HELPSTRING
                 "Choose the type of build, options are:${_conf_string}")

    if(NOT CMAKE_BUILD_TYPE)
        # Encourage the user to specify build type.
        message(STATUS "Setting build type to 'Release' as none was specified.")
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE Release)
    endif()
endif()

# Pick up our CMake modules.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/find-modules)

# Hard dependencies.
find_package(YCM 0.11 REQUIRED)
find_package(YARP 3.7 REQUIRED COMPONENTS os dev)

# SoftNeckControl/SoftArmControl dependencies.
find_package(Eigen3 REQUIRED)
find_package(ROBOTICSLAB_KINEMATICS_DYNAMICS REQUIRED)
find_package(fcontrol REQUIRED)

# Always build YARP devices as MODULE libraries.
set(YARP_FORCE_DYNAMIC_PLUGINS TRUE CACHE INTERNAL "Force dynamic plugins")

# Configure installation paths for YARP resources.
yarp_configure_external_installation(humasoft-yarp-devices WITH_PLUGINS)

# Standard installation directories.
include(GNUInstallDirs)

# Control where libraries and executables are placed during the build.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})

# Enabling coverage.
#option(ENABLE_coverage "Choose if you want to enable coverage collection" OFF)

#if(ENABLE_coverage)
#    # List supported compilers.
#    if(CMAKE_COMPILER_IS_GNUCXX)
#        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
#    endif()
#endif()

# Create targets if specific requirements are satisfied.
include(CMakeDependentOption)

# Add main contents.
add_subdirectory(libraries)
add_subdirectory(programs)
add_subdirectory(share)
add_subdirectory(doc)

# Store the package in the user registry.
#export(PACKAGE HUMASOFT_YARP_DEVICES)

# Create and install config files.
#include(InstallBasicPackageFiles)

# Set exported variables.
#set(HUMASOFT_YARP_DEVICES_BUILD_MODULE_DIR ${CMAKE_SOURCE_DIR}/cmake)
#set(HUMASOFT_YARP_DEVICES_INSTALL_MODULE_DIR ${CMAKE_INSTALL_DATADIR}/HUMASOFT_YARP_DEVICES/cmake)

#install_basic_package_files(HUMASOFT_YARP_DEVICES
#                            VERSION 0.1.0
#                            COMPATIBILITY AnyNewerVersion
#                            NO_SET_AND_CHECK_MACRO
#                            NO_CHECK_REQUIRED_COMPONENTS_MACRO
#                            NAMESPACE HUMASOFT::)

# Configure and create uninstall target.
include(AddUninstallTarget)
