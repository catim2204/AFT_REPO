#ifndef SERIAL_MANAGER_H
#define SERIAL_MANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QByteArray>
#include <QDebug>
#include "Data_Packets.h"
#include <Color_Code_ANSI.h>
#include <QtGlobal>

class SerialManager : public QObject 
{
    Q_OBJECT
    
    //Information being sent and received, own mplementation
    private:
        Telemetry_Package_STR telemetryPackage;
        Information_Package_STR informationPackage;
    //Constructor and destructor functions    
    public:
        explicit SerialManager(QObject *parent = nullptr); //
        ~SerialManager();
    //Event driven, signals emited
    signals:
        void receivedData(const Telemetry_Package_STR &Data);
        void dataSent(const Information_Package_STR &Data);
        void initComplete();
        //void deinitComplete();
    //Event driven, slots executed
    private slots:
        void write(const Information_Package_STR &Data);
        void read();
    //QT implemented objects
    private:
        QByteArray serialBufferSend;
        QByteArray serialBufferReceive;
        QSerialPort *serial;
    private:
        void init();
        void deinit();
    friend class MainWindow;
};





#endif // SERIAL_MANAGER_H
// This header file defines the Serial_Manager class, which is responsible for managing serial communication in the application
// It includes necessary Qt libraries and custom data packet structures.