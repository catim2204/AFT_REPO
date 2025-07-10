#include "ui_mainwindow.h"
#include "Bluetooth_Manager.h"
#include <QElapsedTimer>

BluetoothManager::BluetoothManager(QObject *parent)
    : QObject(parent), discoveryAgent(nullptr), socket(nullptr)
{
    qDebug() << BLU <<"[INFO][BLUETOOTH] Initializing Bluetooth Manager..."<<RESET;
    init();
}

BluetoothManager::~BluetoothManager()
{
    deinit(); // Call deinit to clean up resources
    qDebug() << BLU <<"[INFO][BLUETOOTH] BluetoothManager destroyed."<<RESET;
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

/**
 * @brief Reads bluetooth, copies received data in a buffer and then sends it as a signal 
 * 
 */
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
    emit dataReceived(telemetryPackage);
}

}

/**
 * @brief Function to handle Socket errors.
 * 
 * @param error 
 */
void BluetoothManager::onSocketError(QBluetoothSocket::SocketError error)
{
    qWarning() << "Bluetooth Socket error:" << error << socket->errorString();
}

/**
 *  @brief Function to write data to the Bluetooth Socket.
 *  This function packs the data into a `Information_Package` structure and sends it over the Bluetooth Socket.
 *  @note This function assumes that the `Socket` is already connected and open.
 *  It also assumes that the `Information_Package` structure is defined and matches the
 *  structure expected by the embedded system.
 *  @see Information_Package
 *  @note This function is called by the `CONNECT` button and the `SAVE_STAND` button.
 *  @note This function is called when the `Throttle` slider is moved, if the `Manual Check` checkbox is checked.
 *  @note This function is called when the `ARM Check` checkbox is checked.
 *  @note This function is called when the `CALIBRATE` button is pressed.
 *  @note This function is called when the `START` button is pressed.
 *  @note This function is called when the `SAVE` button is pressed.
 *  @note This function is called when the `Comm_Method` button is pressed to switch between Serial and Bluetooth communication.
 *  @note This function is called when the `CONNECT` button is pressed to establish a Bluetooth connection.
 *  @note This function is called when the `DISCONNECT` button is pressed to close the Bluetooth connection.
 *  @note This function is called when the `SAVE_STAND` button is pressed to save the current settings to the controller. 
*/
void BluetoothManager::write(const Information_Package_STR &Data)
{

        informationPackage= Data; // Copy the data from the parameter to the member variable
        BT_Buffer_Send = QByteArray(reinterpret_cast<const char*>(&informationPackage), sizeof(Information_Package_STR));
        socket->write(BT_Buffer_Send);
        qDebug().noquote()<< GRN << "[INFO][BLUETOOTH]Data sent, size:%ld" << sizeof(Information_Package_STR) <<RESET;
}