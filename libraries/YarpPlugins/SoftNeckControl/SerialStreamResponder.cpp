// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#include "SoftNeckControl.hpp"

#include <cstdlib> // std::atof

#include <yarp/os/Time.h>

using namespace humasoft;

// -----------------------------------------------------------------------------

SerialStreamResponder::SerialStreamResponder(double _timeout)
    : timeout(_timeout),
      localArrivalTime(0.0),
      incl(0.0),
      orient(0.0)
{}

// -----------------------------------------------------------------------------

void SerialStreamResponder::onRead(yarp::os::Bottle & b)
{
    std::lock_guard<std::mutex> lock(mutex);

    if (b.size() == 0)
    {
        return;
    }

    if (accumulateStuff(b.get(0).asString()))
    {
        localArrivalTime = yarp::os::Time::now();
    }
}

// -----------------------------------------------------------------------------

bool SerialStreamResponder::accumulateStuff(const std::string & s)
{
    bool parsed = false;
    std::string::size_type newline = s.find('\n');

    if (newline == std::string::npos)
    {
        accumulator += s;
        return false;
    }

    accumulator += s.substr(0, newline);

    std::string::size_type i = accumulator.find('i');
    std::string::size_type o = accumulator.find('o');

    if (i == 0 && o != std::string::npos && o > i)
    {
        incl = std::atof(accumulator.substr(i + 1, o).c_str());
        orient = std::atof(accumulator.substr(o + 1, accumulator.size()).c_str());
        parsed = true;
    }

    accumulator = s.substr(newline + 1);
    return parsed;
}

// -----------------------------------------------------------------------------

bool SerialStreamResponder::getLastData(double * incl, double * orient)
{
    std::lock_guard<std::mutex> lock(mutex);
    *incl = this->incl;
    *orient = this->orient;
    return yarp::os::Time::now() - localArrivalTime <= timeout;
}

// -----------------------------------------------------------------------------