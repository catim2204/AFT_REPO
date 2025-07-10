/**
 * 
 * @mainpage QT Ui App for Stand Control
 * @section intro_sec Introduction
 * This is the main file of the QT project 
 * @section features_sec Features
 * - Written in C++ 
 * - Uses CMake for bUilding.
 * - Fully documented using Doxygen.
 *
 */

 
 /** 
 * 
 * @file mainwindow.cpp
 * @author Catlin Mihai Timofti (you@domain.com)
 * @brief 
 * @version 2.2
 * @date 2025-06-05
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <algorithm>  
#include <QTimer>
#include <QDir>
#include <qfiledialog.h>

/** @brief This is the constructor, on call it creates the Ui and starts a serial connection on `CONNECT button pressed`*/
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      userInterface(new Ui::MainWindow),
      serialManager(new SerialManager(this)),
      bluetoothManager(new BluetoothManager(this)),
      throttleTimer(new QTimer(this))
{
        Q_ASSERT(userInterface!=nullptr);
        Q_ASSERT(serialManager!=nullptr);
        Q_ASSERT(bluetoothManager != nullptr);
        Q_ASSERT(throttleTimer != nullptr);

        userInterface->setupUi(this);
        connect(userInterface->Comm_Method,&QPushButton::clicked,this,&MainWindow::switchComunications);
        connect(userInterface->CONNECT,&QPushButton::clicked,this,&MainWindow::connectToStand);
        connect(userInterface->SAVE_STAND,&QPushButton::clicked,this,&MainWindow::sendData);
        connect(userInterface->SAVE, &QPushButton::clicked, this, &MainWindow::promptAndSaveLog);
        connect(userInterface->START,&QPushButton::clicked,this,&MainWindow::startTest);
        connect(userInterface->CALIBRATE,&QPushButton::clicked,this,&MainWindow::calibrateStand);
        connect(userInterface->Throttle,&QSlider::valueChanged,this,&MainWindow::sendData);

}
 
MainWindow::~MainWindow() 
{
    delete userInterface;
}

    /** 
    *  @brief This function creates the log based on the received struture
    *   @note This function retreives the log from the `SR_Read` function is called by the `promptAndSaveLog` function
    *   @param filename
    */
void MainWindow::saveLogToCSV(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file for writing:" << filename;
        return;
    }

    QTextStream out(&file);
    // Write header
    out << "Voltage,Current,RPM,Thrust,Torque,Throttle,PWM_Time\n";

    for (const Telemetry_Package_STR &telemetryPackage : std::as_const(Received_Data_Log)) {
        out << telemetryPackage.Voltage_I  << "." 
            << telemetryPackage.Voltage_F  << ","
            << telemetryPackage.Current_I  << "." 
            << telemetryPackage.Current_F  << ","
            << telemetryPackage.RPM_I      << "." 
            << telemetryPackage.RPM_F      << ","
            << telemetryPackage.Thrust_I   << "."
            << telemetryPackage.Thrust_F   << ","
            << telemetryPackage.Torque_I   << "."
            << telemetryPackage.Torque_F   << "," 
            << telemetryPackage.Throttle   << ","
            << telemetryPackage.PWM_Time   <<"\n";
        }

    file.close();
    qDebug().noquote() << "Log saved to" << filename;
}

    /** 
    * @brief On pressing the save button this function allows for saving the log in a selectable location 
    *
    * 
    */
void MainWindow::promptAndSaveLog()
{
    QString filename = QFileDialog::getSaveFileName(
        this,
        "Save CSV Log",
        QDir::homePath() + "/log.csv",               // Default filename
        "CSV Files (*.csv);;All Files (*)"           // File filters
    );

    if (!filename.isEmpty()) 
    {
        saveLogToCSV(filename);  // Call your existing method
    } else 
    {
        qDebug().noquote() << "[INFO] User canceled save dialog.";
    }
}
    /**
    *  @brief This slot initializes serial connection to the microcontroller
    *
    *  @note This function should is called by the `CONNECT` button 
    * 
    */

void MainWindow::displayReceivedData(const Telemetry_Package_STR &data)
{
    Received_Data_Log.append(data);
    userInterface->Voltage->setText(QString("%1.%2")
                         .arg(data.Voltage_I)
                         .arg(data.Voltage_F, 2, 10, QChar('0')));
    userInterface->Current->setText(QString("%1.%2")
                         .arg(data.Current_I)
                         .arg(data.Current_F, 2, 10, QChar('0')));
    userInterface->progressBar->setValue(data.Throttle);
    userInterface->RPM->setText(QString("%1.%2")
                        .arg(data.RPM_I)
                        .arg(data.RPM_F, 2, 10, QChar('0')));
    userInterface->Thrust->setText(QString("%1.%2")
                        .arg(data.Thrust_I)
                        .arg(data.Thrust_F, 3, 10, QChar('0')));
    userInterface->Torque->setText(QString("%1.%2")
                        .arg(data.Torque_I)
                        .arg(data.Torque_F, 3, 10, QChar('0')));
    userInterface->PWM_Time->setText(QString::number(data.PWM_Time));
    userInterface->Error_Screen->display(data.Error_Code);
    qDebug().noquote() << GRN <<"[RECEIVED][MAIN] Struct (" << sizeof(data) << " bytes)"
             << "Voltage_I:"   << data.Voltage_I
             << "Voltage_F:"   << data.Voltage_F
             << "Current_I:"   << data.Current_I
             << "Current_F:"   << data.Current_F
             << "RPM_I:"       << data.RPM_I
             << "RPM_F:"       << data.RPM_F 
             << "Thrust_I:"    << data.Thrust_I
             << "Thrust_F:"    << data.Thrust_F
             << "Torque_I:"    << data.Torque_I
             << "Torque_F:"    << data.Torque_F
             << "Throttle:"    << data.Throttle
             << "PWM_Time:"    << data.PWM_Time
             << "Error_Code:"  << data.Error_Code
             <<RESET;
}
void MainWindow::sendData()
{       
        informationPackage.Start     = 255;
        informationPackage.S_Num   = static_cast<uint8_t>(userInterface->S_Num->value());
        informationPackage.Blade_Count   = static_cast<uint8_t>(userInterface->Blade_Count->value());
        informationPackage.Current_Limit_Steady  = static_cast<uint8_t>(userInterface->CURRENT_LIMIT_STEADY->value());
        if(informationPackage.Arm_Check>0)informationPackage.Throttle  = static_cast<uint8_t>(userInterface->Throttle->value());
        else informationPackage.Throttle=0;
        informationPackage.Arm_Check = userInterface->Arm_Check->isChecked();
        informationPackage.Manual_Check = userInterface->Manul_Check->isChecked();
        informationPackage.Start_Test = start;
        informationPackage.Ui_Mode  = uiMode;
        informationPackage.Save_To_Controller=save;
        informationPackage.Calibration=calibrate;
        informationPackage.Stop      =255;
        informationPackage.Load_Cell_Thrust=static_cast<uint8_t>(userInterface->Load_Cell_Thrust->value());
        informationPackage.Load_Cell_Torque=static_cast<uint8_t>(userInterface->Load_Cell_Torque->value());
        qDebug().noquote()<<GRN
         << "Start"             << informationPackage.Start
         << "Number Of Cells:"  << static_cast<uint8_t> (informationPackage.S_Num)
         << "Current Burst:"    << static_cast<uint8_t> (informationPackage.Blade_Count)
         << "Current Steady:"   << static_cast<uint8_t> (informationPackage.Current_Limit_Steady)
         << "Throttle"          << static_cast<uint8_t> (informationPackage.Throttle)
         << "ARM Check"         << static_cast<uint8_t> (informationPackage.Arm_Check)
         << "Manual Check"      << static_cast<uint8_t> (informationPackage.Manual_Check)
         << "Start State"       << static_cast<uint8_t> (informationPackage.Start_Test)
         << "Ui_Mode"           << static_cast<uint8_t> (informationPackage.Ui_Mode)
         << "Save_To_Controller"<< static_cast<uint8_t> (informationPackage.Save_To_Controller)
         << "Calibration"       << static_cast<uint8_t> (informationPackage.Calibration)
         << "Load cell thrust"  << static_cast<uint8_t> (informationPackage.Load_Cell_Thrust)
         << "Load cell torque"  << static_cast<uint8_t> (informationPackage.Load_Cell_Torque)
         << "Stop"              << informationPackage.Stop
         << RESET;
         emit dataSent(informationPackage);
}

void MainWindow::connectToStand()
{
    userInterface->CONNECT->setText("DISCONNECT");
    switch(commMethod)
    {
    case true:
        serialManager->init();
        bluetoothManager->deinit();
        connect(this,&MainWindow::dataSent,serialManager,&SerialManager::write);
        disconnect(this,&MainWindow::dataSent,bluetoothManager, &BluetoothManager::write); 
        connect(serialManager,&SerialManager::receivedData,this, &MainWindow::displayReceivedData);
       disconnect(bluetoothManager,&BluetoothManager::dataReceived,this, &MainWindow::displayReceivedData);
        qDebug().noquote()<< BLU << "[INFO][MAIN]Comunications switched to serial" << RESET;
        break;
    case false:
        bluetoothManager->init();
        disconnect(this, &MainWindow::dataSent,serialManager,&SerialManager::write);
        connect(this, &MainWindow::dataSent,bluetoothManager, &BluetoothManager::write);
        connect(bluetoothManager,&BluetoothManager::dataReceived,this,&MainWindow::displayReceivedData);
        disconnect(serialManager,&SerialManager::receivedData,this,&MainWindow::displayReceivedData);
        qDebug().noquote()<< BLU << "[INFO][MAIN]Comunications switched to bluetooth" << RESET;
        break;
    default:
        qDebug().noquote()<< RED <<"[ERROR][MAIN]Undefined behaviour"<< RESET;
        break;
    }   
}

void MainWindow::switchComunications()
{
    commMethod = !commMethod;
    qDebug().noquote()<< BLU << "[INFO][MAIN]Bool toggled" << commMethod << RESET;
    disconnect(this, &MainWindow::dataSent,bluetoothManager, &BluetoothManager::write); 
    disconnect(this, &MainWindow::dataSent,serialManager, &SerialManager::write);
    disconnect(serialManager, &SerialManager::receivedData,this, &MainWindow::displayReceivedData);
    disconnect(serialManager, &SerialManager::receivedData,this, &MainWindow::displayReceivedData);
    userInterface->CONNECT->setText("CONNECT");
    emit switchComunicationProtocol(commMethod);
}
void MainWindow::disconnectFromStand()
{
    disconnect(this, &MainWindow::dataSent,bluetoothManager, &BluetoothManager::write); 
    disconnect(this, &MainWindow::dataSent,serialManager, &SerialManager::write);
    disconnect(serialManager, &SerialManager::receivedData,this, &MainWindow::displayReceivedData);
    disconnect(serialManager, &SerialManager::receivedData,this, &MainWindow::displayReceivedData);
    userInterface->CONNECT->setText("CONNECT");
}

void MainWindow::startTest()
{
    start=true;
    sendData();
    start=false;
}
void MainWindow::calibrateStand()
{
    calibrate=true;
    sendData();
    calibrate=false;
}
