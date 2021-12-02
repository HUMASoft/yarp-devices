#include "imudevice.hpp"

void IMUdevice::setupIMU() {

    //This method initializes the IMU, setting it to get euler angles at the desired frequency
    sensor = new IMU3DMGX510(comport, DEFAULT_FREQUENCY); //Main key changing constructor of IMU3DMGX510 atribute string --> string portname = "..."
    if (sensor->check()){
        yarpPort.open(nameyarpoutport+"/sensor:o");
        cout << "Yarp port: " << nameyarpoutport+"/sensor:o" << " has been correctly opened." << endl;
    }

    //IMU will be calibrated at DEFAULT_FREQUENCY
    //sensor->set_freq();
    //sensor->set_IDLEmode();
    //sensor->set_devicetogetgyroacc();
    //sensor->set_streamon();
    //cout << "Calibrating IMU..." << endl;
    //sensor->calibrate();
    //cout << "Calibration done" << endl;

    //Once it has been done, it's prepared to stream euler angles
    sensor->set_freq(frequency);
    //sensor->set_IDLEmode();
    //sensor->set_devicetogetgyroacc();

    //Once the device is correctly connected, it's set to IDLE mode to stop transmitting data till user requests it

    sensor->set_streamon();

}
