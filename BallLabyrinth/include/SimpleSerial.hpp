#pragma once
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

class SimpleSerial
{
public:

    SimpleSerial(const std::string& port, unsigned int baud_rate)
    {
        std::stringstream ss;
        ss << "stty -F " << port << " " << baud_rate
           << " -parenb -parodd cs8 -hupcl -cstopb cread clocal -crtscts -iuclc -ixany -imaxbel "
              "-iutf8 -opost -olcuc -ocrnl -onlcr -onocr -onlret -ofill -ofdel nl0 cr0 tab0 bs0 vt0 ff0 -isig -icanon "
              "-iexten -echo -echoe -echok -echonl -noflsh -xcase -tostop -echoprt -echoctl -echoke";
        std::system(ss.str().c_str());
        serialfd.open(port, std::fstream::in | std::fstream::out);
        std::cout << "setup the device '" << port << "'"<< std::endl;
    }

    void writeString(const std::string& s)
    {
        serialfd.write(s.c_str(), s.size());
        serialfd.flush();
    }

    std::string readLine()
    {
        char buffer[64];
        int bptr = 0;
        /* serialfd.getline(buffer, sizeof(buffer)); */
        std::cout << "last sign of life..." << std::endl;

        /* while (serialfd.readsome(buffer, sizeof(buffer))); */
        int c;
        while ((c = serialfd.get()) && c != std::char_traits<char>::eof() && bptr < sizeof(buffer))
            buffer[bptr++] = c;
        if (bptr < sizeof(buffer))
            buffer[bptr] = '\0';
        buffer[sizeof(buffer) - 1] = '\0';
        serialfd.flush();
        std::cout << buffer << std::endl;
        return std::string(buffer);
    }

private:
    std::fstream serialfd;
};
