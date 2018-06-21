#pragma once
#include "SimpleSerial.hpp"
#include <thread>
#include <mutex>

class SerialCommunication {
private:
    SimpleSerial serial1;
    SimpleSerial serial2;
    bool quit;
    mutable std::mutex mutex;
    double force1;
    double force2;
    double pos1;
    double pos2;
    std::thread thread;
    void run();

public:
    SerialCommunication();
    ~SerialCommunication();

    void communicate();

    double getPos1();
    double getPos2();

    double setForce1();
    double setForce2();
};
