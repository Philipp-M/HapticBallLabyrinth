#include "HandleInterface.hpp"
#include "SerialCommunication.hpp"
#include <boost/lexical_cast.hpp>
#include <iostream>

HandleInterface::HandleInterface(size_t baud, const std::string& dev1, const std::string& dev2)
: quit(false)
, baud(baud)
, dev1(dev1)
, dev2(dev2)
, force1(0)
, force2(0)
, pos1(0)
, pos2(0)
, thread(&HandleInterface::run, this)
{
}

HandleInterface::~HandleInterface() { thread.join(); }

void
HandleInterface::run()
{
#ifdef POSIX
    termios stored_settings;
    tcgetattr(0, &stored_settings);
    termios new_settings = stored_settings;
    new_settings.c_lflag &= (~ICANON);
    new_settings.c_lflag &= (~ISIG);  // don't automatically handle control-C
    tcsetattr(0, TCSANOW, &new_settings);
#endif
    try
    {
        boost::asio::io_service ioService1;
        boost::asio::io_service ioService2;
        // define an instance of the main class of this program
        SerialCommunication c1(
            ioService1,
            boost::lexical_cast<size_t>(baud),
            dev1,
            [this](const char* msg, size_t size) { setPos1(*((int*) msg) / 1000000.0); });
        SerialCommunication c2(
            ioService2,
            boost::lexical_cast<size_t>(baud),
            dev2,
            [this](const char* msg, size_t size) { setPos2(*((int*) msg) / 1000000.0); });
        // run the IO service as a separate thread, so the main thread can block on standard input
        boost::thread t1(boost::bind(&boost::asio::io_service::run, &ioService1));
        boost::thread t2(boost::bind(&boost::asio::io_service::run, &ioService2));
        while (c1.active && c2.active && !quit)  // check the internal state of the connection
                                                     // to make sure it's still running
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
            mutex.lock();
            c1.write(static_cast<uint32_t>(force1 * 1000000));
            c2.write(static_cast<uint32_t>(force2 * 1000000));
            mutex.unlock();
        }
        c1.close();  // close the minicom client connection
        c2.close();  // close the minicom client connection
        t1.join();   // wait for the IO service thread to close
        t2.join();   // wait for the IO service thread to close
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
#ifdef POSIX  // restore default buffering of standard input
    tcsetattr(0, TCSANOW, &stored_settings);
#endif
}

double
HandleInterface::getPos1()
{
    std::lock_guard<std::mutex> lock(mutex);
    return pos1;
}

double
HandleInterface::getPos2()
{
    std::lock_guard<std::mutex> lock(mutex);
    return pos2;
}

void
HandleInterface::setForce1(double force)
{
    std::lock_guard<std::mutex> lock(mutex);
    force1 = force;
}

void
HandleInterface::setForce2(double force)
{
    std::lock_guard<std::mutex> lock(mutex);
    force2 = force;
}

void
HandleInterface::setPos1(double pos)
{
    std::lock_guard<std::mutex> lock(mutex);
    pos1 = pos;
}

void
HandleInterface::setPos2(double pos)
{
    std::lock_guard<std::mutex> lock(mutex);
    pos2 = pos;
}
