yarp_prepare_plugin(SoftNeckControl
                    CATEGORY device
                    TYPE humasoft::SoftNeckControl
                    INCLUDE SoftNeckControl.hpp
                    DEFAULT ON
                    #DEPENDS "EIGEN3_FOUND;ROBOTICSLAB_KINEMATICS_DYNAMICS_FOUND;fcontrol_FOUND"
                    EXTRA_CONFIG WRAPPER=CartesianControlServer)

if(NOT SKIP_SoftNeckControl)

    set(CMAKE_INCLUDE_CURRENT_DIR TRUE) # yarp plugin builder needs this

    yarp_add_plugin(SoftNeckControl SoftNeckControl.hpp
                                    SoftNeckControl.cpp
                                    DeviceDriverImpl.cpp
                                    ICartesianControlImpl.cpp
                                    PeriodicThreadImpl.cpp
                                    IMUSerialStreamResponder.cpp
				     IMU3DMGX510StreamResponder.cpp
                                    MocapStreamResponder.cpp
                                    Kinematics.cpp)

    target_link_libraries(SoftNeckControl YARP::YARP_os
                                          YARP::YARP_dev
                                          Eigen3::Eigen
                                          FControl::fcontrol
                                          ROBOTICSLAB::KinematicsDynamicsInterfaces
                                          ROBOTICSLAB::KinematicRepresentationLib)

    yarp_install(TARGETS SoftNeckControl
                 LIBRARY DESTINATION ${HUMASOFT-YARP-DEVICES_DYNAMIC_PLUGINS_INSTALL_DIR}
                 ARCHIVE DESTINATION ${HUMASOFT-YARP-DEVICES_STATIC_PLUGINS_INSTALL_DIR}
                 YARP_INI DESTINATION ${HUMASOFT-YARP-DEVICES_PLUGIN_MANIFESTS_INSTALL_DIR})
endif()

