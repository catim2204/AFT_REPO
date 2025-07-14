#include "ui_mainwindow.h"
#include "Bluetooth_Manager.h"
#include <QElapsedTimer>

BluetoothManager::BluetoothManager(QObject *parent)
    : QObject(parent), discoveryAgent(nullptr), socket(nullptr)
{
    qDebug() << BLU <<"[INFO][BLUETOOTH] Initializing Bluetooth Manager..."<<RESET;
}

BluetoothManager::~BluetoothManager()
{
    deinit(); 
    if(socket==nullptr)
    qDebug().noquote() << BLU << "[INFO][BLUETOOTH] BluetoothManager destroyed."<<RESET;
    else
    qDebug().noquote() << RED << "[ERROR][BLUETOOTH] Bluetooth failed to deinitialize";
}

void BluetoothManager::init()
{
    // Close and clean up existing Socket
    
    if(socket) 
    {
        if (socket->isOpen()) 
        {
            socket->close();
            qDebug() << YEL <<"[WARNING][BLUETOOTH] Bluetooth Socket closed."<<RESET;
            return;
        }
        socket->deleteLater(); // Safely delete Socket
        socket = nullptr;
    }
    // Stop and clean up discovery agent
    if (discoveryAgent) 
    {
        if (discoveryAgent->isActive()) 
        {
            discoveryAgent->stop();
            qDebug() << "[INFO][BLUETOOTH] Bluetooth discovery stopped.";
            return;
        }
        discoveryAgent->deleteLater(); // Safely delete discovery agent
        discoveryAgent = nullptr;
    }
    // Initialize new socket
    socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this);
    Q_ASSERT(socket != nullptr);
    connect(socket, &QBluetoothSocket::readyRead, this, &BluetoothManager::read);
    //connect(socket, &QBluetoothSocket::errorOccurred, this, &BluetoothManager::onSocketError);

    // Initialize new discovery agent
    discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &BluetoothManager::onDeviceDiscovered);
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, [&] {
        qDebug() << "[INFO][BLUETOOTH] Bluetooth device discovery finished.";
    });
    
    // Start device discovery
    discoveryAgent->start();
    qDebug() << "[INFO][BLUETOOTH] Started Bluetooth device discovery...";
}
void BluetoothManager::deinit()
{
    // Stop and clean up discovery agent
    if (discoveryAgent) 
    {
        if (discoveryAgent->isActive()) 
        {
            discoveryAgent->stop();
            qDebug() << "[INFO][BLUETOOTH] Bluetooth discovery stopped.";
        }
        // Disconnect all signals to prevent callbacks
        discoveryAgent->disconnect();
        discoveryAgent->deleteLater(); // Safely delete
        discoveryAgent = nullptr;
        if (discoveryAgent==nullptr)
        {
            qDebug() << BLU <<"[INFO][BLUETOOTH] Bluetooth discovery agent deinitialized."<<RESET;
        }
        else    
        {
            qDebug().noquote() << RED << "[ERROR][BLUETOOTH]Discovery agent could not be deininitialized, error code"<<discoveryAgent->errorString() <<RESET;
        }   
    }
    // Close and clean up socket
    if(socket) 
    {
        if (socket->isOpen()) {
            socket->close();
            qDebug() << "[INFO][BLUETOOTH] Bluetooth Socket closed.";
        }
        // Disconnect all signals to prevent callbacks
        socket->disconnect();
        socket->deleteLater(); // Safely delete
        socket = nullptr;
        qDebug() << "[INFO][BLUETOOTH] Bluetooth Socket deinitialized.";
    }

    // Update UI if necessary
    qDebug() << "[INFO][BLUETOOTH] Bluetooth deinitialization complete.";
}

void BluetoothManager::onDiscoveryFinished()
{
    qDebug("Discovery Finished");
}

void BluetoothManager::onDeviceDiscovered(const QBluetoothDeviceInfo &device)
{
    qDebug() << "Found device:" << device.name();

    if (device.name() == "ESP32_SPP") {
        qDebug() << "Target device found! Connecting...";
        discoveryAgent->stop();
        const QBluetoothUuid sppUuid(QStringLiteral("00001101-0000-1000-8000-00805F9B34FB"));
        socket->connectToService(device.address(), sppUuid);
    }
}

void BluetoothManager::read()
{   
    QByteArray BT_Buffer_Receive = socket->readAll();
    if (BT_Buffer_Receive.size() == sizeof(Telemetry_Package_STR))
{
    memcpy(&telemetryPackage, BT_Buffer_Receive.constData(), sizeof(Telemetry_Package_STR));
    BT_Buffer_Receive.remove(0, sizeof(Telemetry_Package_STR));
    /**
     * @brief Signal emitted when new data is received via Bluetooth.
     * 
     * @param telemetryPackage The data that has been received.
     */       
    emit receivedData(telemetryPackage);
}

}

void BluetoothManager::onSocketError(QBluetoothSocket::SocketError error)
{
    qDebug() << RED <<"[ERROR][BLUETOOTH]Bluetooth Socket error:" << error << socket->errorString()<<RESET;
}

void BluetoothManager::write(const Information_Package_STR &Data)
{
        BT_Buffer_Send = QByteArray(reinterpret_cast<const char*>(&Data), sizeof(Information_Package_STR));
        socket->write(BT_Buffer_Send);
        qDebug().noquote()<< GRN << "[INFO][BLUETOOTH]Data sent, size:%ld" << sizeof(Information_Package_STR) <<RESET;
}