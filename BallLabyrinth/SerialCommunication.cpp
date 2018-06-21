#include "SerialCommunication.hpp"
#include <iostream>

SerialCommunication::SerialCommunication()
: serial1("/dev/ttyUSB0", 57600)
, serial2("/dev/ttyUSB1", 57600)
, quit(false)
, force1(0)
, force2(0)
, pos1(0)
, pos2(0)
/* , thread(&SerialCommunication::run, this) */ {}

SerialCommunication::~SerialCommunication() { thread.join(); }

void
SerialCommunication::run()
{
    while (!quit)
    {
        communicate();
    }
}


void
SerialCommunication::communicate()
{
    std::cout << "read devices" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(15));
    std::stringstream f1s;
    std::stringstream f2s;
    mutex.lock();
    f1s << "c" << force1 << "\\n";
    f2s << "c" << force2 << "\\n";
    mutex.unlock();
    serial1.writeString(f1s.str());
    serial2.writeString(f2s.str());
    std::this_thread::sleep_for(std::chrono::milliseconds(15));
    mutex.lock();
    pos1 = (double) serial1.readValInt() / 1000000.0;
    pos2 = (double) serial2.readValInt() / 1000000.0;
    mutex.unlock();
    std::cout << "handlepositions: " << pos1 << ", " << pos2 << std::endl;
}

double
SerialCommunication::getPos1()
{
    std::lock_guard<std::mutex> lock(mutex);
    return pos1;
}
double
SerialCommunication::getPos2()
{
    std::lock_guard<std::mutex> lock(mutex);
    return pos2;
}

double
SerialCommunication::setForce1()
{
    std::lock_guard<std::mutex> lock(mutex);
    return force1;
}
double
SerialCommunication::setForce2()
{
    std::lock_guard<std::mutex> lock(mutex);
    return force2;
}
