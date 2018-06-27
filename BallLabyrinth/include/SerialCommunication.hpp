#pragma once

#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/thread.hpp>
#include <deque>
#include <iostream>

#ifdef POSIX
#include <termios.h>
#endif

class SerialCommunication
{
public:
    SerialCommunication(boost::asio::io_service&                 ioService,
                        unsigned int                             baud,
                        const std::string&                       device,
                        std::function<void(const char*, size_t)> readCallback)
    : active(true), ioService(ioService), serialPort(ioService, device), readCallback(readCallback)
    {
        if (!serialPort.is_open())
        {
            std::cerr << "Failed to open serial port\n";
            return;
        }
        boost::asio::serial_port_base::baud_rate baud_option(baud);
        serialPort.set_option(baud_option);  // set the baud rate after the port has been opened
        readStart();
    }

    void write(int32_t val)  // pass the write data to the doWrite function via the io service
                             // in the other thread
    {
        ioService.post(boost::bind(&SerialCommunication::doWrite, this, val));
    }

    void close()  // call the doClose function via the io service in the other thread
    {
        ioService.post(
            boost::bind(&SerialCommunication::doClose, this, boost::system::error_code()));
    }

    bool active;  // remains true while this object is still operating

private:
    static const int maxReadLength = 512;  // maximum amount of data to read in one operation

    void readStart(void)
    {  // Start an asynchronous read and call readComplete when it completes or fails
        serialPort.async_read_some(boost::asio::buffer(readMsg, maxReadLength),
                                   boost::bind(&SerialCommunication::readComplete,
                                               this,
                                               boost::asio::placeholders::error,
                                               boost::asio::placeholders::bytes_transferred));
    }

    void readComplete(const boost::system::error_code& error, size_t bytes_transferred)
    {  // the asynchronous read operation has now completed or failed and returned an error
        if (!error)
        {  // read completed, so process the data
            readCallback(readMsg, bytes_transferred);
            /* std::cout << *((int*) readMsg) / 1000000.0 << std::endl; */
            /* readStart();  // start waiting for another asynchronous read again */
        }
        else
            doClose(error);
    }

    void doWrite(int32_t val)
    {  // callback to handle write call from outside this class
        bool write_in_progress = !writeMsg.empty();  // is there anything currently being written?
        for (size_t i = 0; i < sizeof(int32_t); ++i)
            writeMsg.push_back(((char*) &val)[i]);
        if (!write_in_progress)  // if nothing is currently being written, then start
            write_start();
    }

    void write_start(void)
    {  // Start an asynchronous write and call writeComplete when it completes or fails
        if (writeMsg.size() >= 4)
        {
            char buffer[4];
            buffer[0] = writeMsg.back();
            writeMsg.pop_back();
            buffer[1] = writeMsg.back();
            writeMsg.pop_back();
            buffer[2] = writeMsg.back();
            writeMsg.pop_back();
            buffer[3] = writeMsg.back();
            writeMsg.pop_back();
            boost::asio::async_write(serialPort,
                                     boost::asio::buffer(buffer, 4),
                                     boost::bind(&SerialCommunication::writeComplete,
                                                 this,
                                                 boost::asio::placeholders::error));
            /* std::this_thread::sleep_for(std::chrono::milliseconds(7)); */
        }
    }

    void writeComplete(const boost::system::error_code& error)
    {  // the asynchronous read operation has now completed or failed and returned an error
        if (!error)
        {                           // write completed, so send next write data
            /* writeMsg.pop_front();   // remove the completed data */
            if (!writeMsg.empty())  // if there is anthing left to be written
                write_start();      // then start sending the next item in the buffer
            readStart();
        }
        else
            doClose(error);
    }

    void doClose(const boost::system::error_code& error)
    {  // something has gone wrong, so close the socket & make this object inactive
        if (error == boost::asio::error::operation_aborted)  // if this call is the result of a
                                                             // timer cancel()
            return;  // ignore it because the connection cancelled the timer
        /* if (error) */
        /*     std::cerr << "Error: " << error.message() << std::endl;  // show the error message */
        /* else */
        /*     std::cerr << "Error: Connection did not succeed.\n"; */
        serialPort.close();
        active = false;
    }

private:
    boost::asio::io_service& ioService;   // the main IO service that runs this connection
    boost::asio::serial_port serialPort;  // the serial port this instance is connected to
    char                     readMsg[maxReadLength];  // data read from the socket
    std::deque<char>         writeMsg;                // buffered write data
    std::function<void(const char*, size_t)> readCallback;
};
