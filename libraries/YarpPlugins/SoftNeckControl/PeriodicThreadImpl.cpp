﻿// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#include "SoftNeckControl.hpp"

#include <cmath> // std::isnormal

#include <KinematicRepresentation.hpp>
#include <yarp/os/LogStream.h>

//In order to clasify the system, we are using fstream library
#include <fstream>

using namespace humasoft;
using namespace roboticslab::KinRepresentation;

// ------------------- PeriodicThread Related ------------------------------------

void SoftNeckControl::run()
{
    switch (getCurrentState())
    {
    case VOCAB_CC_MOVJ_CONTROLLING:
        if(controlType=="ioCoupled"){
            yInfo() <<"Starting Inclination Orientation Coupled Contro";
            !sensorPort.isClosed() ? handleMovjClosedLoopIOCoupled() : handleMovjOpenLoop();
        }
        else if(controlType=="ioUncoupled"){
            yInfo() <<"Starting Inclination Orientation Uncoupled Control";
            !sensorPort.isClosed() ? handleMovjClosedLoopIOUncoupled() : handleMovjOpenLoop();
        }
        else if(controlType=="rpUncoupled"){
            yInfo() <<"Starting Roll Pitch Uncoupled Control";

            !sensorPort.isClosed() ? handleMovjClosedLoopRPUncoupled() : handleMovjOpenLoop();
        }
        else if(controlType=="newControl"){
            yInfo() <<"Starting NEW Control";

            !sensorPort.isClosed() ? handleMovjNewClosedLoop() : handleMovjOpenLoop();
        }
        else yInfo() <<"Control mode not defined. Running in open loop...";
        break;
    default:
        break;
    }
}

// -----------------------------------------------------------------------------

void SoftNeckControl::handleMovjOpenLoop()
{
    bool done;

    if (!iPositionControl->checkMotionDone(&done))
    {
        yError() <<"Unable to query current robot state.";
        cmcSuccess = false;
        stopControl();
        return;
    }

    if (done)
    {
        setCurrentState(VOCAB_CC_NOT_CONTROLLING);

        if (!iPositionControl->setRefSpeeds(qRefSpeeds.data()))
        {
            yWarning() <<"setRefSpeeds (to restore) failed.";
        }
    }
}

// -----------------------------------------------------------------------------

void SoftNeckControl::handleMovjClosedLoopIOCoupled()
{
    std::vector<double> x_imu;
    double polarError,
           azimuthError,
           polarCs,
           azimuthCs
           = 0.0;

    switch (sensorType) {
        case '0':
            if (!serialStreamResponder->getLastData(x_imu))
            {
                yWarning() <<"Outdated SparkfunIMU serial stream data.";
            } break;
        case '1':
            if (!immu3dmgx510StreamResponder->getLastData(x_imu))
            {
                yWarning() <<"Outdated 3DMGX510IMU stream data.";
            } break;
        case '2':
            if (!mocapStreamResponder->getLastData(x_imu))
            {
                yWarning() <<"Outdated Mocap stream data.";
                iVelocityControl->stop();
            } break;
    }

    std::vector<double> xd = targetPose;   
    polarError   = xd[0] - x_imu[0];
    azimuthError = xd[1] - x_imu[1];

    if(std::abs(azimuthError)> 180 )
    {
        if(azimuthError > 0)
            azimuthError = azimuthError - 360.0;
        else
            azimuthError = azimuthError + 360.0;
    }

    // controlamos siempre en inclinación
    polarCs   = polarError   > *controllerPolar;
    if (!std::isnormal(polarCs))
    {
        polarCs = 0.0;
    }

    xd[0] = polarCs;

    /* control en orientacion solo si:
     * (inclinacion > 5)
     */
    if(targetPose[0]>5)
    {
        yInfo() <<"> Controlando en orientación";
        azimuthCs = azimuthError > *controllerAzimuth;

        if (!std::isnormal(azimuthCs))
        {
            azimuthCs = 0.0;
        }

        xd[1] = azimuthCs;
    }

    yDebug("- Polar:   target %f, sensor %f, error %f, cs: %f\n", targetPose[0], x_imu[0], polarError, polarCs);
    yDebug("- Azimuth: target %f, sensor %f, error %f, cs: %f\n", targetPose[1], x_imu[1], azimuthError, azimuthCs);

    if (!encodePose(xd, xd, coordinate_system::NONE, orientation_system::POLAR_AZIMUTH, angular_units::DEGREES))
    {
        yError() <<"encodePose failed.";
        cmcSuccess = false;
        stopControl();
        return;
    }

    if (!sendTargets(xd))
    {
        yWarning() <<"Command error, not updating control this iteration.";
    }
}

// -----------------------------------------------------------------------------

void SoftNeckControl::handleMovjClosedLoopIOUncoupled()
{
    std::vector<double> x_imu;
    double polarError,
           azimuthError,
           polarCs,
           azimuthCs
           = 0.0;

    double cs1; // motor izq
    double cs2; // motor der
    std::vector<int> m; // motor izq, der, tercero
    std::vector<double> cs;
    int area_c, area_d = 0; // currect area, destination area (area_p = area actual)
    cs.resize(3);

    switch (sensorType) {
        case '0':
            if (!serialStreamResponder->getLastData(x_imu))
            {
                yWarning() <<"Outdated serial stream data.";
                iVelocityControl->stop();
            } break;
        case '1':
            if (!immu3dmgx510StreamResponder->getLastData(x_imu))
            {
                yWarning() <<"Outdated IMU 3dmgx510 stream data.";
                iVelocityControl->stop();
            } break;
        case '2':
            if (!mocapStreamResponder->getLastData(x_imu))
            {
                yWarning() <<"Outdated Mocap stream data.";
                iVelocityControl->stop();
            } break;
    }

    std::vector<double> xd = targetPose;
    polarError   = (xd[0] - x_imu[0]);
    azimuthError = (xd[1] - x_imu[1]);

    if (x_imu[1]<120)  area_c=1;
    else if (x_imu[1]>240) area_c=3;
    else area_c=2;

    // Paso por cero
    if (xd[1]<120) // Area 1
    {
        area_d=1;
        m={0,1,2}; //m={1,2,0}; config anterior
        if (area_c==3) azimuthError+=360;
    }
    else if (xd[1]>240) // Area 3
    {        
        area_d=3;
        m={2,0,1}; //m={0,1,2};
        if (area_c==1) azimuthError-=360;
    }
    else // Area 2
    {
        area_d=2;
        m={1,2,0}; //m={2,0,1};
    }

    polarCs   = polarError*M_1_PI/180   > *incon;
    azimuthCs = azimuthError*M_1_PI/180 > *orcon;

    if (!std::isnormal(polarCs)) polarCs = 0;
    if (!std::isnormal(azimuthCs) || x_imu[1] <5) azimuthCs = 0;

    //ajustar solo orientación, hasta llegar al área
    if (area_d!=area_c && x_imu[0]>5) polarCs=0;

    cs[0]=(polarCs-azimuthCs); // motor izquierdo del area
    cs[1]=(polarCs+azimuthCs); //motor derecho del area
    cs[2]= - (cs[0]+cs[1])/2; //2/3

    //si no estoy en el área
    if ((area_d!=area_c) && (x_imu[0]>5)){
        if (azimuthError>0) // aumentar de area bloqueo iz
        {
            cs[0]=0;
            cs[2]=-cs[1];
        }

        else
        {
            cs[1]=0;
            cs[2]=-cs[0];
        }
    }

    printf("> sensor(i%f o%f) motors (%f %f %f)\n",x_imu[0], x_imu[1], cs[0], cs[1], cs[2]);
    if (!iVelocityControl->velocityMove(3,m.data(),cs.data()));
    {
        //yError() <<"velocityMove failed.\n");
    }
}

void SoftNeckControl::handleMovjClosedLoopRPUncoupled(){

    double rollError,
            pitchError,
            rollCs,
            pitchCs
            = 0.0;

    std::vector<double> x_imu;
    std::vector<double> xd(2);
    //std::vector<double> xd = targetPose;

    switch (sensorType) {
        case '1':
            if (!immu3dmgx510StreamResponder->getLastData(x_imu))
            {
                yWarning() <<"Outdated IMU 3dmgx510 stream data.";
                iPositionControl->stop();
            } break;
        case '2':
            if (!mocapStreamResponder->getLastData(x_imu))
            {
                yWarning() <<"Outdated Mocap stream data.";
                iPositionControl->stop();
            } break;
    }

    // transform [roll.pitch] to [inc, ori]
    std::vector<double> io_imu(2); // inc, ori
    //io_imu[0] = sqrt(pow(x_imu[1], 2) + pow(x_imu[0], 2));
    //io_imu[1] = fmod( (360 - (atan2(-x_imu[0], x_imu[1])) * 180/M_PI), 360);

    rollError = targetPose[0] - x_imu[0];
    pitchError = targetPose[1] - x_imu[1];

    //Control process
    rollCs = controllerRollFracc->OutputUpdate(rollError);
    if (!std::isnormal(rollCs))
    {
        rollCs = 0.0;
    }
    xd[0] = rollCs;

    pitchCs = controllerPitchFracc->OutputUpdate(pitchError);
    if (!std::isnormal(pitchCs))
    {
        pitchCs = 0.0;
    }
    xd[1] = pitchCs;

    double p1 = - 0.001*(xd[1] / 1.5);
    double p2 = - 0.001*( (xd[0] / 1.732) - (xd[1] / 3) );
    double p3 = - 0.001*( (xd[1] / -3) - (xd[0] / 1.732) );

    if (p3<0){
        p3=p3*0.5;
    }
    if (p2<0){
        p2=p2*0.5;
    }
    if (p1<0){
        p1=p1*0.5;
    }


    tprev = tnow;
    tnow = std::chrono::system_clock::now();
    chrono::nanoseconds elapsedNanoseconds = tprev.time_since_epoch()-tnow.time_since_epoch();

    double totaltime = elapsedNanoseconds.count();

    cout << "-----------------------------\n" << endl;
    cout << "Total time: ms " << (totaltime/1000000) << endl;
    //cout << "Inclination: " << io_imu[0] << "  Orientation: " << io_imu[1] << endl;
    cout << "Roll: " << x_imu[0] << "  Pitch: " << x_imu[1] << endl;
    cout << "-> RollTarget: " << targetPose[0] << " PitchTarget:  " << targetPose[1] << " >>>>> RollError: " << rollError << "  PitchError: " << pitchError << endl;
    cout << "-> Motor positions >>>>> P1: " << p1 << " P2: " << p2 << " P3: " << p3 << endl;
    cout << "-----------------------------\n" << endl;

    std::vector<double> qd={p1,p2,p3};

    if (!iPositionControl->positionMove(qd.data()))
    {
        yError() <<"positionMove failed.";
    }

    //Uncomment it to receive data from testing
    //testingFile << yarp::os::Time::now()*numtime << "," << targetPose[0] << "," << targetPose[1] << "," << x_imu[0] << "," << x_imu[1]<< endl;
    numtime = numtime+1;

}

void SoftNeckControl::handleMovjNewClosedLoop(){
    // new control

}
