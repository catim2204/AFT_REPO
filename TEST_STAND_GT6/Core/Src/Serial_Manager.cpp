#include <Serial_Manager.h>
#include "ui_mainwindow.h"
#include <QMessageBox>
SerialManager::SerialManager(QObject *parent): QObject(parent), serial(new QSerialPort(this))
{
    qDebug() << BLU <<"[INFO][SERIAL] Initializing Serial Manager..." << RESET;
}
SerialManager::~SerialManager()
{   
    if (serial->isOpen()) 
    {
        serial->close();
        qDebug() << BLU <<"[INFO][SERIAL]Serial port closed." << RESET;
    }
    serial = nullptr;
    delete serial;
    if(serial==nullptr)
    qDebug().noquote() << BLU <<"[INFO][SERIAL]SerialManager destroyed." << RESET;
    else
    qDebug().noquote() << RED <<"[ERROR][SERIAL]Serial could not be terminated" << serial->errorString()<<RESET;
}

void SerialManager::write(const Information_Package_STR &Data)
{
    Q_ASSERT(serial != nullptr);
    informationPackage = Data; // Copy the data from the parameter to the member variable
    qDebug() << BLU <<"[INFO][SERIAL] Preparing to send data to Serial port:" << sizeof(Information_Package_STR) << "bytes"<< RESET;
    // Convert the struct to a QByteArray for sending
    // Ensure that the struct is packed correctly to avoid padding issues
    serialBufferSend.clear(); // Clear the buffer before sending new data
    // Use memcpy to copy the struct data into the QByteArray
    // This ensures that the data is sent in the correct format
    serialBufferSend.resize(sizeof(Information_Package_STR));
    memcpy(serialBufferSend.data(), &informationPackage, sizeof(Information_Package_STR));
    // Send the data through the serial port
    if(serial->isOpen())
    {
        qint64 bytesWritten = serial->write(serialBufferSend);
         if (bytesWritten == -1) 
        {   
            qDebug().noquote() << RED <<"[ERROR][SERIAL] Failed to write data to Serial port:" << serial->errorString()<<RESET;
        } else 
        {
            qDebug().noquote() << BLU <<"[INFO][SERIAL] Sent data to Serial port:" << bytesWritten << "bytes" <<RESET;
        }
    } else 
    {
        qDebug().noquote() << RED <<"[ERROR][SERIAL]Serial port is not open. Cannot send data." << RESET ;
    }
    // Emit signal to notify that data has been sent
    serialBufferSend = QByteArray(reinterpret_cast<const char*>(&informationPackage), sizeof(Information_Package_STR));
    emit dataSent(informationPackage);
}

void SerialManager::read()
{
    Q_ASSERT(serial != nullptr);
    // Read incoming bytes from the serial port into a buffer
    serialBufferReceive.append(serial->readAll());
    qDebug() << BLU <<"[INFO][SERIAL] Received data from Serial port:" << serialBufferReceive.size() << "bytes" <<RESET;
    // Check if we have enough data for a full Telemetry_Package_STR
    while (serialBufferReceive.size() >= sizeof(Telemetry_Package_STR)) 
    {
        // Extract the struct from the buffer
        Telemetry_Package_STR telemetryData;
        memcpy(&telemetryData, serialBufferReceive.constData(), sizeof(Telemetry_Package_STR));
        
        // Remove the processed bytes from the buffer
        serialBufferReceive.remove(0, sizeof(Telemetry_Package_STR));
        
        // Emit signal with the received data
        emit receivedData(telemetryData);
    }
}


void SerialManager::init()
{
    if(serial->isOpen())
    {   
        qDebug().noquote()<< YEL <<"[WARNING][SERIAL]Serial port already open! Returning!" << RESET;
        return;    
    }   
    serial->setPortName("/dev/ttyACM1");
    serial->setBaudRate(QSerialPort::Baud115200);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    connect(serial, &QSerialPort::readyRead, this,&SerialManager::read);
    if(serial->open(QIODevice::ReadWrite))
    {
        qDebug().noquote()<< BLU <<("[INFO][SERIAL]Serial initialization complete")<< RESET;
    }
    else
    {
        qDebug().noquote()<< RED <<"[ERROR][SERIAL]Serial port initialization failed!"<<serial->errorString()<<RESET ;
        return;
    }
    Q_ASSERT(serial != nullptr);
}

