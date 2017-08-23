//======================================================================//
//  This software is free: you can redistribute it and/or modify        //
//  it under the terms of the GNU General Public License Version 3,     //
//  as published by the Free Software Foundation.                       //
//  This software is distributed in the hope that it will be useful,    //
//  but WITHOUT ANY WARRANTY; without even the implied warranty of      //
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE..  See the      //
//  GNU General Public License for more details.                        //
//  You should have received a copy of the GNU General Public License   //
//  Version 3 in the file COPYING that came with this distribution.     //
//  If not, see <http://www.gnu.org/licenses/>                          //
//======================================================================//
//                                                                      //
//      Copyright (c) 2017 Oswaldo Peña and Saith Rodriguez             //
//      Universidad Santo Tomas - Bogota, Colombia                      //
//      oswaldopena@usantotomas.edu.co                                  //
//      saithrodriguez@usantotomas.edu.co                               //
//                                                                      //
//======================================================================//


#ifndef QMYO_H
#define QMYO_H

#include <QObject>
#include <QTimer>
#include <QEventLoop>

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QQuaternion>

#include <iostream>

#include "myohw.h"

QT_FORWARD_DECLARE_CLASS(QBluetoothDeviceDiscoveryAgent)
QT_FORWARD_DECLARE_CLASS(QBluetoothDeviceInfo)

QT_USE_NAMESPACE

class QMyo : public QObject
{
    
    Q_OBJECT

public:
    class ImuData 
    {
	public:
	    double orientationQuaternion[4];
	    double orientationEuler[3];
	    double accelerometer[3];
	    double gyroscope[3];
    };
    
    class EmgData
    {
	public:
	    char samples1[8];
	    char samples2[8];
    };

public:   
    QMyo();
    virtual ~QMyo();
    void connect(QBluetoothAddress address);
    
    void createControlService();
    myohw_fw_info_t readMyoInfo();
    QString readMyoInfoString();
    myohw_fw_version_t readFirmwareVersion();
    QString readFirmwareVersionString();
    bool writeCommandSetMode(uint8_t emgMode, uint8_t imuMode, uint8_t classifierMode);
    bool writeCommandVibrate(uint8_t vibrate);
    bool writeCommandDeepSleep();
    bool writeCommandVibrate2(uint16_t duration1, uint8_t strength1, uint16_t duration2, uint8_t strength2, uint16_t duration3, uint8_t strength3, uint16_t duration4, uint8_t strength4, uint16_t duration5, uint8_t strength5, uint16_t duration6, uint8_t strength6);
    bool writeCommandSetSleepMode(uint8_t sleepMode);
    bool writeCommandUnlock(uint8_t unlockType);
    bool writeCommandUserAction(uint8_t classifierModelType);
    
    void createImuDataService();
    void createEmgDataService();
    void createClassifierDataService();
    
private:
    QEventLoop *eventLoop;
    QLowEnergyController *controller;
    bool foundMyo;
    
    QLowEnergyService *controlService;
    QLowEnergyCharacteristic myoInfoChar;
    QLowEnergyCharacteristic firmwareVersionChar;
    QLowEnergyCharacteristic commandChar;
    QByteArray readResult;
    bool writeResult;
    
    QLowEnergyService *imuDataService;
    QLowEnergyCharacteristic imuDataChar;
    QLowEnergyCharacteristic motionEventChar;
    
    QLowEnergyService *emgDataService;
    QLowEnergyCharacteristic emgData0Char;
    QLowEnergyCharacteristic emgData1Char;
    QLowEnergyCharacteristic emgData2Char;
    QLowEnergyCharacteristic emgData3Char;
    
    QLowEnergyService *classifierDataService;
    QLowEnergyCharacteristic classifierMotionEventChar;
    
    QByteArray byteNotification = QByteArray::fromHex("0100");
    QByteArray byteIndication = QByteArray::fromHex("0200");
    
    QBluetoothUuid generateUuid(quint16 shortUuid);
    
private slots:
    void myoConnected();
    void myoDisconnected();
    void controllerError(QLowEnergyController::Error error);
    void serviceDiscovered(const QBluetoothUuid &uuid);
    void discoveryFinished();
    
    void serviceError(QLowEnergyService::ServiceError error);
    void controlServiceStateChanged(QLowEnergyService::ServiceState state);
    void characteristicRead(QLowEnergyCharacteristic characteristic, QByteArray data);
    void characteristicWritten(QLowEnergyCharacteristic characteristic, QByteArray data);
    
    void imuDataServiceStateChanged(QLowEnergyService::ServiceState state);
    void imuDataServiceCharacteristicChanged(QLowEnergyCharacteristic characteristic, QByteArray data);
    
    void emgDataServiceStateChanged(QLowEnergyService::ServiceState state);
    void emgDataServiceCharacteristicChanged(QLowEnergyCharacteristic characteristic, QByteArray data);
    
    void classifierDataServiceStateChanged(QLowEnergyService::ServiceState state);
    void classifierDataServiceCharacteristicChanged(QLowEnergyCharacteristic characteristic, QByteArray data);
signals:
    void myoStatus(QString status);
    void serviceStatus(QString status);
    void updateImuData(QMyo::ImuData imuData);
    void updateEmgData(QMyo::EmgData emgData);
};

#endif // QMYO_H
