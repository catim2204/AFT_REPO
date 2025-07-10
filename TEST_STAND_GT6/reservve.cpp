#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("TEST_STAND_GT6", "Main");

    return app.exec();
}
// void MainWindow::initSerial()
// {

//     serial->setPortName("/dev/ttyACM0");
//     serial->setBaudRate(QSerialPort::Baud115200);
//     serial->setDataBits(QSerialPort::Data8);
//     serial->setParity(QSerialPort::NoParity);
//     serial->setStopBits(QSerialPort::OneStop);
//     serial->setFlowControl(QSerialPort::NoFlowControl);
//     Ui_MODE=true;
//       if (serial->open(QIODevice::ReadWrite)) 
//     {
//        qDebug().noquote() << "[INFO] Serial port opened.";
//        connect(serial, &QSerialPort::readyRead, this, &MainWindow::serialRead);
//        userInterface->CONNECT->setText("DISCONNECT");
//     } 
//       else 
//     {
//        qDebug().noquote() << "[ERROR] Failed to open serial port:" << serial->errorString();
//        disconnect(serial, &QSerialPort::readyRead, this, &MainWindow::serialRead);
//     }
// }
// void MainWindow::deinitSerial()
// {
//     if (serial) {
//         // Close the serial port if open
//         if (serial->isOpen()) {
//             serial->close();
//             qDebug().noquote() << "[INFO] Serial port closed.";
//         }

//         // Disconnect all signals to prevent callbacks
//         serial->disconnect();
//         qDebug().noquote() << "[INFO] Serial port signals disconnected.";

//         // Reset state and Ui
//         Ui_MODE = false;
//         userInterface->CONNECT->setText("CONNECT");
//         qDebug().noquote() << "[INFO] Serial port deinitialization complete.";
//     }
// }
// void MainWindow::initBluetooth()
// {
//     // Close and clean up existing socket
//     if(socket) 
//     {
//         if (socket->isOpen()) 
//         {
//             socket->close();
//             qDebug().noquote() << "[INFO] Bluetooth socket closed.";
//             return;
//         }
//         socket->deleteLater(); // Safely delete socket
//         socket = nullptr;
//     }

//     // Stop and clean up discovery agent
//     if (discoveryAgent) 
//     {
//         if (discoveryAgent->isActive()) 
//         {
//             discoveryAgent->stop();
//             qDebug().noquote() << "[INFO] Bluetooth discovery stopped.";
//             return;
//         }
//         discoveryAgent->deleteLater(); // Safely delete discovery agent
//         discoveryAgent = nullptr;
//     }

//     // Initialize new socket
//     socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this);
//     connect(socket, &QBluetoothSocket::connected, this, [&] 
//     {
//         qDebug().noquote() << "Bluetooth connected!";
//         userInterface->CONNECT->setText("DISCONNECT");
//     });
//     connect(socket, &QBluetoothSocket::disconnected, this, [&] 
//     {
//         qDebug().noquote() << "Bluetooth disconnected.";
//         userInterface->CONNECT->setText("CONNECT");
//     });
//     connect(socket, &QBluetoothSocket::readyRead, bluetoothManager, &BluetoothManager::read);
//     //connect(socket, &QBluetoothSocket::errorOccurred, this, &MainWindow::onSocketError);

//     // Initialize new discovery agent
//     discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
//     connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &MainWindow::onDeviceDiscovered);
//     connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, [&] {
//         qDebug().noquote() << "[INFO] Bluetooth device discovery finished.";
//     });

//     // Start device discovery
//     discoveryAgent->start();
//     qDebug().noquote() << "[INFO] Started Bluetooth device discovery...";
// }
// void MainWindow::deinitBluetooth()
// {
//     // Stop and clean up discovery agent
//     if (discoveryAgent) {
//         if (discoveryAgent->isActive()) 
//         {
//             discoveryAgent->stop();
//             qDebug().noquote() << "[INFO] Bluetooth discovery stopped.";
//         }
//         // Disconnect all signals to prevent callbacks
//         discoveryAgent->disconnect();
//         discoveryAgent->deleteLater(); // Safely delete
//         discoveryAgent = nullptr;
//         qDebug().noquote() << "[INFO] Bluetooth discovery agent deinitialized.";
//     }

//     // Close and clean up socket
//     if (socket) {
//         if (socket->isOpen()) {
//             socket->close();
//             qDebug().noquote() << "[INFO] Bluetooth socket closed.";
//         }
//         // Disconnect all signals to prevent callbacks
//         socket->disconnect();
//         socket->deleteLater(); // Safely delete
//         socket = nullptr;
//         qDebug().noquote() << "[INFO] Bluetooth socket deinitialized.";
//     }

//     // Update Ui if necessary
//     userInterface->CONNECT->setText("CONNECT");
//     qDebug().noquote() << "[INFO] Bluetooth deinitialization complete.";
// }

// void MainWindow::onDiscoveryFinished()
// {
//     qDebug("Discovery Finished");
// }

// void MainWindow::onDeviceDiscovered(const QBluetoothDeviceInfo &device)
// {
//     qDebug().noquote() << "Found device:" << device.name();

//     if (device.name() == "ESP32_SPP") {
//         qDebug().noquote() << "Target device found! Connecting...";
//         discoveryAgent->stop();
//         const QBluetoothUuid sppUuid(QStringLiteral("00001101-0000-1000-8000-00805F9B34FB"));
//         socket->connectToService(device.address(), sppUuid);
//     }
// }
// /**
//  * @brief Slot called when data is received from the Bluetooth socket.
//  *
//  * This function reads incoming bytes from the Bluetooth socket into a buffer.
//  * When a full `Telemetry_Package_STR` structure has been received, it is extracted,
//  * parsed, and displayed in the Ui (voltage, current, throttle, etc.).
//  * 
//  * Also outputs the full structure content to the debug console for verification.
//  *
//  * @note This function assumes that the incoming data stream is well-aligned 
//  *       and matches the exact size and layout of `Telemetry_Package_STR`, which corresponds to the embedded "Sent_Data_STR".
//  *
//  * @see Telemetry_Package_STR
//  */
// void MainWindow::bluetoothRead()
// {
//     // Ensure socket is valid before using it
//     if (!socket) return;   
    
//     QByteArray BT_Buffer_Receive = socket->readAll();

//     qDebug().noquote() << "[INFO] Received data from Bluetooth socket:" << BT_Buffer_Receive.size() << "bytes";
//     qDebug().noquote() << "[INFO] Received data:" << BT_Buffer_Receive.toHex();
//     while (BT_Buffer_Receive.size() >= 10 * sizeof(Telemetry_Package_STR)) 
//     {   
     
//         memcpy(&telemetryPackage, BT_Buffer_Receive.constData(), sizeof(Telemetry_Package_STR));
//         Received_Data_Log.append(telemetryPackage);
//         BT_Buffer_Receive.remove(0, sizeof(Telemetry_Package_STR));
//         // Display the received data in the Ui
//         // Assuming telemetryPackage is a struct with the same layout as Telemetry_Package_STR
//         // Convert the struct to a QByteArray for debugging
//         // and display the values in the Ui
//         // Assuming telemetryPackage is a struct with the same layout as Telemetry_Package_STR  
//         QByteArray byteArray(reinterpret_cast<const char*>(&telemetryPackage), sizeof(telemetryPackage));
//         userInterface->Voltage->setText(QString("%1.%2")
//                              .arg(telemetryPackage.Voltage_I)
//                              .arg(telemetryPackage.Voltage_F, 2, 10, QChar('0')));

//         userInterface->Current->setText(QString("%1.%2")
//                              .arg(telemetryPackage.Current_I)
//                              .arg(telemetryPackage.Current_F, 2, 10, QChar('0')));

//         userInterface->progressBar->setValue(telemetryPackage.Throttle);
//         userInterface->RPM->setText(QString("%1.%2")
//                  .arg(telemetryPackage.RPM_I)
//                  .arg(telemetryPackage.RPM_F, 2, 10, QChar('0')));

//         userInterface->Thrust->setText(QString("%1.%2")
//                             .arg(telemetryPackage.Thrust_I)
//                             .arg(telemetryPackage.Thrust_F, 3, 10, QChar('0')));

//         userInterface->Torque->setText(QString("%1.%2")
//                             .arg(telemetryPackage.Torque_I)
//                             .arg(telemetryPackage.Torque_F, 3, 10, QChar('0')));

//         userInterface->PWM_Time->setText(QString::number(telemetryPackage.PWM_Time));
//         userInterface->Error_Screen->display(telemetryPackage.Error_Code);

//         qDebug().noquote() << "[RECEIVED] Struct (" << byteArray.size() << " bytes)"
//                  << "Voltage_I:"   << telemetryPackage.Voltage_I
//                  << "Voltage_F:"   << telemetryPackage.Voltage_F
//                  << "Current_I:"   << telemetryPackage.Current_I
//                  << "Current_F:"   << telemetryPackage.Current_F
//                  << "RPM_I:"      << telemetryPackage.RPM_I
//                  << "RPM_F:"      << telemetryPackage.RPM_F 
//                  << "Thrust_I:"    << telemetryPackage.Thrust_I
//                  << "Thrust_F:"    << telemetryPackage.Thrust_F
//                  << "Torque_I:"    << telemetryPackage.Torque_I
//                  << "Torque_F:"    << telemetryPackage.Torque_F
//                  << "Throttle:"    << telemetryPackage.Throttle
//                  << "PWM_Time:"    << telemetryPackage.PWM_Time
//                  << "Error_Code"   << telemetryPackage.Error_Code;
//     }
// }

// /**
//  * @brief Function to handle socket errors.
//  * 
//  * @param error 
//  */
// void MainWindow::onSocketError(QBluetoothSocket::SocketError error)
// {
//     qWarning() << "Bluetooth socket error:" << error << socket->errorString();
// }

// /**
//  * @brief Function to write data to the Bluetooth socket.
//  * * This function packs the data into a `Sent_Data_STR` structure and sends it over the Bluetooth socket.
//  *  * @note This function assumes that the `socket` is already connected and open.
//  * * It also assumes that the `Sent_Data_STR` structure is defined and matches the
//  * * structure expected by the embedded system.
//  * * @see Sent_Data_STR
//  * * @note This function is called by the `CONNECT` button and the `SAVE_STAND` button.
//  * * @note This function is called when the `Throttle` slider is moved, if the `Manual Check` checkbox is checked.
//  * * @note This function is called when the `ARM Check` checkbox is checked.
//  * * @note This function is called when the `CALIBRATE` button is pressed.
//  * * @note This function is called when the `START` button is pressed.
//  * * @note This function is called when the `SAVE` button is pressed.
//  * * @note This function is called when the `Comm_Method` button is pressed to switch between Serial and Bluetooth communication.
//  * * @note This function is called when the `CONNECT` button is pressed to establish a Bluetooth connection.
//  * * @note This function is called when the `DISCONNECT` button is pressed to close the Bluetooth connection.
//  * * @note This function is called when the `SAVE_STAND` button is pressed to save the current settings to the controller.
//  * 
//  * 
//  * 
//  * 
//  */
// void MainWindow::bluetoothWrite()
// {

// if (socket->isOpen() && socket->isWritable()) 
//     {
//         informationPackage.Start     = 255;
//         informationPackage.S_Num   = static_cast<uint8_t>(userInterface->S_Num->value());
//         informationPackage.Blade_Count   = static_cast<uint8_t>(userInterface->Blade_Count->value());
//         informationPackage.Current_Limit_Steady  = static_cast<uint8_t>(userInterface->CURRENT_LIMIT_STEADY->value());
//         if(informationPackage.Arm_Check>0)informationPackage.Throttle  = static_cast<uint8_t>(userInterface->Throttle->value());
//         else informationPackage.Throttle=0;
//         informationPackage.Arm_Check = userInterface->Arm_Check->isChecked();
//         informationPackage.Manual_Check = userInterface->Manul_Check->isChecked();
//         informationPackage.Start_Test = START_B;
//         informationPackage.Ui_Mode  = Ui_MODE;
//         informationPackage.Save_To_Controller=0;
//         informationPackage.Calibration=1;
//         informationPackage.Stop      =255;
//         informationPackage.Load_Cell_Thrust=static_cast<u_int8_t>(userInterface->Load_Cell_Thrust->value());
//         informationPackage.Load_Cell_Torque=static_cast<u_int8_t>(userInterface->Load_Cell_Torque->value());
//         // Assume informationPackage is already populated
//         BT_Buffer_Send = QByteArray(reinterpret_cast<const char*>(&informationPackage), sizeof(Information_Package_STR));
//         socket->write(BT_Buffer_Send);
//         qDebug().noquote() << "[SEND] Sent Data struct (" << BT_Buffer_Send.size() << " bytes)";
//         qDebug().noquote() 
//          << "Start"             << informationPackage.Start
//          << "Number Of Cells:"  << static_cast<u_int8_t> (informationPackage.S_Num)
//          << "Current Burst:"    << static_cast<u_int8_t> (informationPackage.Blade_Count)
//          << "Current Steady:"   << static_cast<u_int8_t> (informationPackage.Current_Limit_Steady)
//          << "Throttle"          << static_cast<u_int8_t> (informationPackage.Throttle)
//          << "ARM Check"         << static_cast<u_int8_t> (informationPackage.Arm_Check)
//          << "Manual Check"      << static_cast<u_int8_t> (informationPackage.Manual_Check)
//          << "Start State"       << static_cast<u_int8_t> (informationPackage.Start_Test)
//          << "Ui_Mode"           << static_cast<u_int8_t> (informationPackage.Ui_Mode)
//          << "Save_To_Controller"<< static_cast<u_int8_t> (informationPackage.Save_To_Controller)
//          << "Calibration"       << static_cast<u_int8_t> (informationPackage.Calibration)
//          << "Stop"              << informationPackage.Stop;

//     } 
//     else 
//     {
//         qDebug().noquote() << "[ERROR] Socket not open or not writable.";
//     }
// }



// /**
//  * @brief Slot should be called whenever the `Send_Data_STR` is needed to send data to the MCU.
//  *  This data includes operational limits, button presses, status of the checkboxes and throttle position.
//  *  The function packs and sends the information as described in the header file
//  * 
//  * @note This function reqUieres data structured exactly how it is in the  in order to work properly.
//  * It also needs to match the data structure in the corresponding MCU `Telemetry_Package_STR` 
//  *  
//  * @see Sent_Data_STR
//  */
// void MainWindow::serialWrite()
// { 
//     if (serial->isOpen() && serial->isWritable()) 
//     {
//         informationPackage.Start     = 255;
//         informationPackage.S_Num   = static_cast<uint8_t>(userInterface->S_Num->value());
//         informationPackage.Blade_Count   = static_cast<uint8_t>(userInterface->Blade_Count->value());
//         informationPackage.Current_Limit_Steady  = static_cast<uint8_t>(userInterface->CURRENT_LIMIT_STEADY->value());
//         if(informationPackage.Arm_Check>0)informationPackage.Throttle  = static_cast<uint8_t>(userInterface->Throttle->value());
//         else informationPackage.Throttle=0;
//         informationPackage.Arm_Check = userInterface->Arm_Check->isChecked();
//         informationPackage.Manual_Check = userInterface->Manul_Check->isChecked();
//         informationPackage.Start_Test = START_B;
//         informationPackage.Ui_Mode  = Ui_MODE;
//         informationPackage.Save_To_Controller=0;
//         informationPackage.Calibration=1;
//         informationPackage.Stop      =255;
//         informationPackage.Load_Cell_Thrust=static_cast<uint8_t>(userInterface->Load_Cell_Thrust->value());
//         informationPackage.Load_Cell_Torque=static_cast<uint8_t>(userInterface->Load_Cell_Torque->value());
//         // Assume informationPackage is already populated
//         SR_Buffer_Send = QByteArray(reinterpret_cast<const char*>(&informationPackage), sizeof(Information_Package_STR));
//         serial->write(SR_Buffer_Send);
//         qDebug().noquote() << "[SEND] Sent Data struct (" << SR_Buffer_Send.size() << " bytes)";
//         qDebug().noquote() 
//          << "Start"             << informationPackage.Start
//          << "Number Of Cells:"  << static_cast<uint8_t> (informationPackage.S_Num)
//          << "Current Burst:"    << static_cast<uint8_t> (informationPackage.Blade_Count)
//          << "Current Steady:"   << static_cast<uint8_t> (informationPackage.Current_Limit_Steady)
//          << "Throttle"          << static_cast<uint8_t> (informationPackage.Throttle)
//          << "ARM Check"         << static_cast<uint8_t> (informationPackage.Arm_Check)
//          << "Manual Check"      << static_cast<uint8_t> (informationPackage.Manual_Check)
//          << "Start State"       << static_cast<uint8_t> (informationPackage.Start_Test)
//          << "Ui_Mode"           << static_cast<uint8_t> (informationPackage.Ui_Mode)
//          << "Save_To_Controller"<< static_cast<uint8_t> (informationPackage.Save_To_Controller)
//          << "Calibration"       << static_cast<uint8_t> (informationPackage.Calibration)
//          << "Stop"              << informationPackage.Stop;

//     } 
//     else 
//     {
//         qDebug().noquote() << "[ERROR] Serial port not open or not writable.";
//     }
// }

// /**
//  * @brief Slot called when serial data is available.
//  *
//  * This function reads incoming bytes from the serial port into a buffer.
//  * When a full `Telemetry_Package_STR` structure has been received, it is extracted,
//  * parsed, and displayed in the Ui (voltage, current, throttle, etc.).
//  * 
//  * Also outputs the full structure content to the debug console for verification.
//  *
//  * @note This function assumes that the incoming data stream is well-aligned 
//  *       and matches the exact size and layout of `Telemetry_Package_STR`, which corresponds to the embedded "Sent_Data_STR".
//  *
//  * @see Telemetry_Package_STR
//  */
// void MainWindow::serialRead()
// {   
//     SR_Buffer_Receive.append(serial->readAll());
//     while (SR_Buffer_Receive.size() >= sizeof(Telemetry_Package_STR)) 
//     {
//         Telemetry_Package_STR data;
//         memcpy(&telemetryPackage, SR_Buffer_Receive.constData(), sizeof(Telemetry_Package_STR));
//         Received_Data_Log.append(telemetryPackage);
//         SR_Buffer_Receive.remove(0, sizeof(Telemetry_Package_STR));
//         QByteArray byteArray(reinterpret_cast<const char*>(&telemetryPackage), sizeof(telemetryPackage));
//         userInterface->Voltage->setText(QString("%1.%2")
//                             .arg(telemetryPackage.Voltage_I)
//                             .arg(telemetryPackage.Voltage_F, 2, 10, QChar('0')));
//         userInterface->Current->setText(QString("%1.%2")
//                             .arg(telemetryPackage.Current_I)
//                             .arg(telemetryPackage.Current_F, 2, 10, QChar('0')));
//         userInterface->progressBar->setValue(telemetryPackage.Throttle);
//         userInterface->RPM->setText(QString("%1.%2")
//                          .arg(telemetryPackage.RPM_I)
//                          .arg(telemetryPackage.RPM_F, 2, 10, QChar('0')));
//         userInterface->Thrust->setText(QString("%1.%2")
//                             .arg(telemetryPackage.Thrust_I)
//                             .arg(telemetryPackage.Thrust_F, 3, 10, QChar('0')));
//         userInterface->Torque->setText(QString("%1.%2")
//                             .arg(telemetryPackage.Torque_I)
//                             .arg(telemetryPackage.Torque_F, 3, 10, QChar('0')));
//         userInterface->PWM_Time->setText(QString::number(telemetryPackage.PWM_Time));
//         userInterface->Error_Screen->display(telemetryPackage.Error_Code);
//         // Show the received data
        
//         qDebug().noquote() << "[RECEIVED] Received Data struct (" << byteArray.size() << " bytes)"
//                  << "Voltage_I:"        << telemetryPackage.Voltage_I
//                  << "Voltage_F:"        << telemetryPackage.Voltage_F
//                  << "Current_I:"        << telemetryPackage.Current_I
//                  << "Current_F:"        << telemetryPackage.Current_F
//                  << "RPM_I:"            << telemetryPackage.RPM_I
//                  << "RPM_F:"            << telemetryPackage.RPM_F
//                  << "Thrust_I:"         << telemetryPackage.Thrust_I
//                  << "Thrust_F:"         << telemetryPackage.Thrust_F
//                  << "Torque_I:"         << telemetryPackage.Torque_I
//                  << "Torque_F:"         << telemetryPackage.Torque_F
//                  << "Throttle:"         << telemetryPackage.Throttle
//                  << "PWM_Time"          << telemetryPackage.PWM_Time;
//     }
        

// }