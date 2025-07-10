/**
 * @file mainwindow.h
 * @author Timofti Mihai Catalin (you@domain.com)
 * @brief 
 * @version 2.2
 * @date 2025-06-05
 * 
 * @copyright Copyright (c) 2025
 * 
 */


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QList>
#include <QFile>
#include <QTextStream>
#include <QIODevice>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QBluetoothSocket>
#include <QBluetoothUuid>
#include <Data_Packets.h>
#include <QTimer>
#include <QtGlobal>

#include <Color_Code_ANSI.h>

#include <Bluetooth_Manager.h>
#include <Serial_Manager.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{   
    Q_OBJECT
private:
    bool start;
    bool arm;
    bool manual;
    bool uiMode;
    bool commMethod;
    bool save; 
    bool calibrate;
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void dataSent(const Information_Package_STR &Data);
    void switchComunicationProtocol(bool Comm_Data);

private slots:
    void displayReceivedData(const Telemetry_Package_STR &data);
    void sendData();
    void connectToStand();
    void disconnectFromStand();
    void switchComunications();
    void startTest();
    void calibrateStand();

private:
    void saveLogToCSV(const QString& filename);
    Ui::MainWindow* userInterface;
    QTimer *sendTimer;
    QList<Telemetry_Package_STR> Received_Data_Log;  // Store all received structs
    QTimer* throttleTimer;
    void promptAndSaveLog();  Information_Package_STR informationPackage;
    Telemetry_Package_STR telemetryPackage;
    BluetoothManager *bluetoothManager = nullptr;
    SerialManager *serialManager = nullptr; 

};
#endif // MAINWINDOW_H

 