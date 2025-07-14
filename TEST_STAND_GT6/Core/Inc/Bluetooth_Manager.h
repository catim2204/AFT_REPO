#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H


#include <QObject>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QBluetoothSocket>
#include <QBluetoothUuid>
#include "Data_Packets.h"
#include "Color_Code_ANSI.h"
#include <QtGlobal>

class BluetoothManager : public QObject 
{
    Q_OBJECT

private:

    Telemetry_Package_STR telemetryPackage;
    Information_Package_STR informationPackage;
    
public:
    explicit BluetoothManager(QObject *parent = nullptr); // Updated constructor
    ~BluetoothManager();

signals:
    void deviceDiscovered(const QBluetoothDeviceInfo &Info);
    void discoveryFinished();
    void receivedData(const Telemetry_Package_STR &Data);
    void socketErrorOccurred(QBluetoothSocket::SocketError Error);

public slots:
    void init();
    void deinit();
    void write(const Information_Package_STR &Data);
    void read();

private slots:
    void onSocketError(QBluetoothSocket::SocketError Error);
    void onDeviceDiscovered(const QBluetoothDeviceInfo &Device);
    void onDiscoveryFinished();

private:
    QBluetoothDeviceDiscoveryAgent *discoveryAgent;
    QBluetoothSocket *socket;
    QByteArray BT_Buffer_Send;
    QByteArray BT_Buffer_Receive;
};

#endif // BLUETOOTH_MANAGER_H
