#pragma once
#include <boost/asio.hpp>

class SimpleSerial
{
public:
    /**
     * Constructor.
     * \param port device name, example "/dev/ttyUSB0" or "COM4"
     * \param baud_rate communication speed, example 9600 or 115200
     * \throws boost::system::system_error if cannot open the
     * serial device
     */
    SimpleSerial(const std::string& port, unsigned int baud_rate)
    : io()
    , serial(io, port)
    {
        serial.set_option(boost::asio::serial_port_base::baud_rate(baud_rate));
    }

    /**
     * Write a string to the serial device.
     * \param s string to write
     * \throws boost::system::system_error on failure
     */
    void writeString(const std::string& s)
    {
        boost::asio::write(serial, boost::asio::buffer(s.c_str(), s.size()));
    }

    /**
     * Blocks until a line is received from the serial device.
     * Eventual '\n' or '\r\n' characters at the end of the string are removed.
     * \return a string containing the received line
     * \throws boost::system::system_error on failure
     */
    std::string readLine()
    {
        // Reading data char by char, code is optimized for simplicity, not speed
        using namespace boost;
        char        c;
        std::string result;
        for (;;)
        {
            asio::read(serial, asio::buffer(&c, 1));
            switch (c)
            {
                case '\r':
                    break;
                case '\n':
                    return result;
                default:
                    result += c;
            }
        }
    }

    int readValInt()
    {
        using namespace boost;
        int retVal;
        char buffer[4];
        asio::read(serial, asio::buffer(buffer, 4));
        ((char*)&retVal)[0] = buffer[0];
        ((char*)&retVal)[1] = buffer[1];
        ((char*)&retVal)[2] = buffer[2];
        ((char*)&retVal)[3] = buffer[3];
        /* ((char*)&retVal)[3] = buffer[4]; */
        /* ((char*)&retVal)[2] = buffer[5]; */
        /* ((char*)&retVal)[1] = buffer[6]; */
        /* ((char*)&retVal)[0] = buffer[7]; */
        return retVal;
    }

private:
    boost::asio::io_service  io;
    boost::asio::serial_port serial;
};
