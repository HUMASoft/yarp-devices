#Creation of our new device from mocapdevice.h and MOCAPdevice class within it
yarp_prepare_plugin(Mocap
                    CATEGORY device
                    TYPE MOCAPdevice
                    INCLUDE mocapdevice.hpp
                    DEFAULT ON)

if(NOT SKIP_Mocap)


    set(CMAKE_INCLUDE_CURRENT_DIR TRUE) # yarp plugin builder needs this
    set (NATNET ${CMAKE_CURRENT_SOURCE_DIR}/natnet-integration)	# headers
    message("NATNET: " ${NATNET})

    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lNatNetLibShared")
    find_library (NATNET_LIB NatNetLibShared)		# finding libNatNetLibShared.so in /usr/local/lib
    message("NATNET_LIB: " ${NATNET_LIB})


    include_directories(${NATNET})

    yarp_add_plugin(Mocap                             mocapdevice.hpp
                                                      Mocap.cpp
                                                      DeviceDriverMocapImpl.cpp
                                                      PeriodicThreadMOCAP.cpp
                                                      )

    target_link_libraries(Mocap                       YARP::YARP_os
                                                      YARP::YARP_dev
                                                      YARP::YARP_sig
                                                      YARP::YARP_init #key line to make network work
                                                      ${NATNET_LIB}
                                                      )

    yarp_install(TARGETS Mocap
                                      LIBRARY DESTINATION ${HUMASOFT-YARP-DEVICES_DYNAMIC_PLUGINS_INSTALL_DIR}
                                      ARCHIVE DESTINATION ${HUMASOFT-YARP-DEVICES_STATIC_PLUGINS_INSTALL_DIR}
                                      YARP_INI DESTINATION ${HUMASOFT-YARP-DEVICES_PLUGIN_MANIFESTS_INSTALL_DIR})


endif()
