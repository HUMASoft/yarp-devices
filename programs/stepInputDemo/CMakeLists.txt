cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

# Start a project.
project(stepInputDemo LANGUAGES CXX)

find_package(YARP 3.2 REQUIRED COMPONENTS OS dev)
find_package(COLOR_DEBUG REQUIRED)
find_package(ROBOTICSLAB_KINEMATICS_DYNAMICS REQUIRED)

# Set up our main executable.
add_executable(stepInputDemo stepInputDemo.cpp)

#INCLUDE_DIRECTORIES (${SUBDIR_INCLUDE_DIRECTORIES}) #Sensor-integration lib included

target_link_libraries(stepInputDemo YARP::YARP_OS
                                        YARP::YARP_init
                                        YARP::YARP_dev
                                        ROBOTICSLAB::KinematicsDynamicsInterfaces
                                        ROBOTICSLAB::ColorDebug)

#target_link_libraries(stepInputDemo SensorIntegration) #Sensor-integration lib included

include(GNUInstallDirs)

install(TARGETS stepInputDemo
        DESTINATION ${CMAKE_INSTALL_BINDIR})
