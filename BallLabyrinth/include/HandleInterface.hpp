#pragma once
#include <thread>
#include <mutex>

class HapticForceManager;

class HandleInterface {
private:
    size_t baud;
    std::string dev1;
    std::string dev2;
    double force1;
    double force2;
    double pos1;
    double pos2;
    std::thread thread;
    mutable std::recursive_mutex mutex;
    HapticForceManager* hapticForceManager;
    void run();

    void setPos1(double pos);
    void setPos2(double pos);

public:
    HandleInterface(size_t baud, const std::string& dev1, const std::string& dev2);
    ~HandleInterface();

    double getPos1();
    double getPos2();

    double getForce1();
    double getForce2();

    void setForce1(double force);
    void setForce2(double force);

    void setHapticForceManager(HapticForceManager* manager);

    bool quit;
};
