#include "include/QMyo.h"

QMyo::QMyo()
{
    controller = NULL;
    foundMyo = false;
    controlService = NULL;
    imuDataService = NULL;
    emgDataService = NULL;
    classifierDataService = NULL;
    eventLoop = new QEventLoop();
    writeResult = false;
}

QMyo::~QMyo()
{
    
}

void QMyo::connect(QBluetoothAddress address)
{
    if(controller) 
    {
        controller->disconnectFromDevice();
        delete controller;
        controller = NULL;
    }
    controller = new QLowEnergyController(address, this);
    QObject::connect(controller, SIGNAL(connected()), this, SLOT(myoConnected()));
    QObject::connect(controller, SIGNAL(disconnected()), this, SLOT(myoDisconnected()));
    QObject::connect(controller, SIGNAL(error(QLowEnergyController::Error)), this, SLOT(controllerError(QLowEnergyController::Error)));
    QObject::connect(controller, SIGNAL(serviceDiscovered(QBluetoothUuid)), this, SLOT(serviceDiscovered(QBluetoothUuid)));
    QObject::connect(controller, SIGNAL(discoveryFinished()), this, SLOT(discoveryFinished()));
    
    foundMyo = false;
    controller->connectToDevice();
}


//Connect and Disconnect
void QMyo::myoConnected()
{
    controller->discoverServices();
}

void QMyo::myoDisconnected()
{
    controller->disconnectFromDevice();
    delete controller;
    controller = NULL;
    emit myoStatus("DISCONNECTED");
}


//GenerateService
QBluetoothUuid QMyo::generateUuid(quint16 shortUuid)
{
    static const uint8_t MyoServiceBaseUuid[] = MYO_SERVICE_INFO_UUID;
    quint128 uuid;
    for(int i=0; i<16; i++)
	uuid.data[i] = MyoServiceBaseUuid[15-i];
    uuid.data[2] = quint8((shortUuid >> 8)  & 0xFF);
    uuid.data[3] = quint8(shortUuid & 0xFF);
    return QBluetoothUuid(uuid);
}


//Services Discovered
void QMyo::serviceDiscovered(const QBluetoothUuid &uuid)
{
    if(uuid == generateUuid(ControlService)) 
	foundMyo = true;
}

void QMyo::discoveryFinished()
{
    if(foundMyo)
	emit myoStatus("CONNECTED");
    else
	emit myoStatus("NOT_A_MYO");
}


//Errors
void QMyo::controllerError(QLowEnergyController::Error error)
{
    emit myoStatus("ERROR");
}

void QMyo::serviceError(QLowEnergyService::ServiceError error)
{
    printf("No");
    if(error == QLowEnergyService::CharacteristicWriteError)
    {
	writeResult = false;
	eventLoop->quit();
    }
}


//Services
//ControlService
void QMyo::createControlService()
{
    delete controlService;
    controlService = NULL;
    controlService = controller->createServiceObject(generateUuid(ControlService));
    if(!controlService) 
    {
	emit serviceStatus("CONTROL_SERVICE_NOT_FOUND");
        return;
    }

    QObject::connect(controlService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(controlServiceStateChanged(QLowEnergyService::ServiceState)));
    QObject::connect(controlService, SIGNAL(characteristicRead(QLowEnergyCharacteristic,QByteArray)), this, SLOT(characteristicRead(QLowEnergyCharacteristic,QByteArray)));
    QObject::connect(controlService, SIGNAL(characteristicWritten(QLowEnergyCharacteristic,QByteArray)), this, SLOT(characteristicWritten(QLowEnergyCharacteristic,QByteArray)));
    QObject::connect(controlService, SIGNAL(error(QLowEnergyService::ServiceError)), this, SLOT(serviceError(QLowEnergyService::ServiceError)));
    controlService->discoverDetails();
}

void QMyo::controlServiceStateChanged(QLowEnergyService::ServiceState state)
{
    switch(state) 
    {
	case QLowEnergyService::ServiceDiscovered:
	{
	    myoInfoChar = controlService->characteristic(generateUuid(MyoInfoCharacteristic));
	    if (!myoInfoChar.isValid()) 
	    {
		emit serviceStatus("MYO_INFO_CHARACTERISTIC_NOT_FOUND");
		break;
	   
	    }
	    firmwareVersionChar = controlService->characteristic(generateUuid(FirmwareVersionCharacteristic));
	    if (!firmwareVersionChar.isValid()) 
	    {
		emit serviceStatus("FIRMWARE_VERSION_CHARACTERISTIC_NOT_FOUND");
		break;
	   
	    }
	    commandChar = controlService->characteristic(generateUuid(CommandCharacteristic));
	    if (!commandChar.isValid()) 
	    {
		emit serviceStatus("COMMAND_CHARACTERISTIC_NOT_FOUND");
		break;
	   
	    }
	    emit serviceStatus("CONTROL_SERVICE_CREATED");
	    break;
	}
	default:
	    //nothing for now
	    break;
    }
}

void QMyo::characteristicRead(QLowEnergyCharacteristic characteristic, QByteArray data)
{
    readResult = data;
    eventLoop->quit();
}

void QMyo::characteristicWritten(QLowEnergyCharacteristic characteristic, QByteArray data)
{
    writeResult = true;
    eventLoop->quit();
}


//Imu Service
void QMyo::createImuDataService()
{
    delete imuDataService;
    imuDataService = NULL;
    imuDataService = controller->createServiceObject(generateUuid(ImuDataService));
    if(!imuDataService) 
    {
	emit serviceStatus("IMU_DATA_SERVICE_NOT_FOUND");
        return;
    }

    QObject::connect(imuDataService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(imuDataServiceStateChanged(QLowEnergyService::ServiceState)));
    QObject::connect(imuDataService, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)), this, SLOT(imuDataServiceCharacteristicChanged(QLowEnergyCharacteristic,QByteArray)));
    QObject::connect(imuDataService, SIGNAL(error(QLowEnergyService::ServiceError)), this, SLOT(serviceError(QLowEnergyService::ServiceError)));
    imuDataService->discoverDetails();
}

void QMyo::imuDataServiceStateChanged(QLowEnergyService::ServiceState state)
{
    switch(state) 
    {
	case QLowEnergyService::ServiceDiscovered:
	{
	    imuDataChar = imuDataService->characteristic(generateUuid(IMUDataCharacteristic));
	    if (!imuDataChar.isValid()) 
	    {
		emit serviceStatus("IMU_DATA_CHARACTERISTIC_NOT_FOUND");
		break;
	   
	    }
	    QLowEnergyDescriptor dataNotification = imuDataChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
	    if (dataNotification.isValid()) 
	    {
		imuDataService->writeDescriptor(dataNotification, byteNotification);
	    }
	    
	    
	    motionEventChar = imuDataService->characteristic(generateUuid(MotionEventCharacteristic));
	    if (!motionEventChar.isValid()) 
	    {
		emit serviceStatus("MOTION_EVENT_CHARACTERISTIC_NOT_FOUND");
		break;
	   
	    }
	    QLowEnergyDescriptor motionEventIndication = motionEventChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
	    if (motionEventIndication.isValid()) 
	    {
		imuDataService->writeDescriptor(motionEventIndication, byteIndication);
	    }
	    emit serviceStatus("IMU_DATA_SERVICE_CREATED");
	    break;
	}
	default:
	    //nothing for now
	    break;
    }
}

void QMyo::imuDataServiceCharacteristicChanged(QLowEnergyCharacteristic characteristic, QByteArray data)
{
    if(characteristic.uuid() == generateUuid(IMUDataCharacteristic))
    {
	float w = (float)((int16_t)((((uint8_t)(data.at(1))<<8)&0xFF00) + (uint8_t)(data.at(0))))/(float)(MYOHW_ORIENTATION_SCALE);
	float x = (float)((int16_t)((((uint8_t)(data.at(3))<<8)&0xFF00) + (uint8_t)(data.at(2))))/(float)(MYOHW_ORIENTATION_SCALE);
	float y = (float)((int16_t)((((uint8_t)(data.at(5))<<8)&0xFF00) + (uint8_t)(data.at(4))))/(float)(MYOHW_ORIENTATION_SCALE);
	float z = (float)((int16_t)((((uint8_t)(data.at(7))<<8)&0xFF00) + (uint8_t)(data.at(6))))/(float)(MYOHW_ORIENTATION_SCALE);	
	QMyo::ImuData imuData; 
	imuData.orientationQuaternion[0] = w;
	imuData.orientationQuaternion[1] = x;
	imuData.orientationQuaternion[2] = y;
	imuData.orientationQuaternion[3] = z;
	
	float roll = atan2(2.0f * (w*x + y*z), 1.0f - 2.0f * (x*x + y*y));
        float pitch = asin(qMax(-1.0f, qMin(1.0f, (2.0f * (w*y - z*x)))));
	double yaw = atan2(2.0f * (w*z + x*y), 1.0f - 2.0f * (y*y + z*z));
	imuData.orientationEuler[0] = atan2(2.0f * (w*x + y*z), 1.0f - 2.0f * (x*x + y*y));
	imuData.orientationEuler[1] = asin(qMax(-1.0f, qMin(1.0f, (2.0f * (w*y - z*x)))));
	imuData.orientationEuler[2] = atan2(2.0f * (w*z + x*y), 1.0f - 2.0f * (y*y + z*z));
	
	imuData.accelerometer[0] = (float)((int16_t)((((uint8_t)(data.at(9))<<8)&0xFF00) + (uint8_t)(data.at(8))))/(float)(MYOHW_ACCELEROMETER_SCALE);
	imuData.accelerometer[1] = (float)((int16_t)((((uint8_t)(data.at(11))<<8)&0xFF00) + (uint8_t)(data.at(10))))/(float)(MYOHW_ACCELEROMETER_SCALE);
	imuData.accelerometer[2] = (float)((int16_t)((((uint8_t)(data.at(13))<<8)&0xFF00) + (uint8_t)(data.at(12))))/(float)(MYOHW_ACCELEROMETER_SCALE);
	
	imuData.gyroscope[0] = (float)((int16_t)((((uint8_t)(data.at(15))<<8)&0xFF00) + (uint8_t)(data.at(14))))/(float)(MYOHW_GYROSCOPE_SCALE);
	imuData.gyroscope[1] = (float)((int16_t)((((uint8_t)(data.at(17))<<8)&0xFF00) + (uint8_t)(data.at(16))))/(float)(MYOHW_GYROSCOPE_SCALE);
	imuData.gyroscope[2] = (float)((int16_t)((((uint8_t)(data.at(19))<<8)&0xFF00) + (uint8_t)(data.at(18))))/(float)(MYOHW_GYROSCOPE_SCALE);
	
	emit updateImuData(imuData);
    }
    else
    {
	//printf("Motion Event\n");
    }
    /*for(int i=0; i<data.size(); i++)
	printf("%X ", (uint8_t)(data.at(i)));
    printf("\n");*/
}


//Emg Service
void QMyo::createEmgDataService()
{
    delete emgDataService;
    emgDataService = NULL;
    emgDataService = controller->createServiceObject(generateUuid(EmgDataService));
    if(!emgDataService) 
    {
	emit serviceStatus("EMG_DATA_SERVICE_NOT_FOUND");
        return;
    }
    
    QObject::connect(emgDataService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(emgDataServiceStateChanged(QLowEnergyService::ServiceState)));
    QObject::connect(emgDataService, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)), this, SLOT(emgDataServiceCharacteristicChanged(QLowEnergyCharacteristic,QByteArray)));
    QObject::connect(emgDataService, SIGNAL(error(QLowEnergyService::ServiceError)), this, SLOT(serviceError(QLowEnergyService::ServiceError)));
    
    emgDataService->discoverDetails();
}

void QMyo::emgDataServiceStateChanged(QLowEnergyService::ServiceState state)
{
    switch(state)
    {
	case QLowEnergyService::ServiceDiscovered:
	{
	    emgData0Char = emgDataService->characteristic(generateUuid(EmgData0Characteristic));
	    if (!emgData0Char.isValid()) 
	    {
		emit serviceStatus("EMG_DATA0_CHARACTERISTIC_NOT_FOUND");
		break;
	   
	    }
	    QLowEnergyDescriptor emgData0Notification = emgData0Char.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
	    if (emgData0Notification.isValid()) 
	    {
		emgDataService->writeDescriptor(emgData0Notification, byteNotification);
	    }
	    
	    
	    emgData1Char = emgDataService->characteristic(generateUuid(EmgData1Characteristic));
	    if (!emgData1Char.isValid()) 
	    {
		emit serviceStatus("EMG_DATA1_CHARACTERISTIC_NOT_FOUND");
		break;
	   
	    }
	    QLowEnergyDescriptor emgData1Notification = emgData1Char.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
	    if (emgData1Notification.isValid()) 
	    {
		emgDataService->writeDescriptor(emgData1Notification, byteNotification);
	    }
	    
	    
	    emgData2Char = emgDataService->characteristic(generateUuid(EmgData2Characteristic));
	    if (!emgData2Char.isValid()) 
	    {
		emit serviceStatus("EMG_DATA2_CHARACTERISTIC_NOT_FOUND");
		break;
	   
	    }
	    QLowEnergyDescriptor emgData2Notification = emgData2Char.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
	    if (emgData2Notification.isValid()) 
	    {
		emgDataService->writeDescriptor(emgData2Notification, byteNotification);
	    }
	    
	    
	    emgData3Char = emgDataService->characteristic(generateUuid(EmgData3Characteristic));
	    if (!emgData3Char.isValid()) 
	    {
		emit serviceStatus("EMG_DATA3_CHARACTERISTIC_NOT_FOUND");
		break;
	   
	    }
	    QLowEnergyDescriptor emgData3Notification = emgData3Char.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
	    if (emgData3Notification.isValid()) 
	    {
		emgDataService->writeDescriptor(emgData3Notification, byteNotification);
	    }
	    emit serviceStatus("EMG_DATA_SERVICE_CREATED");
	    break;
	}
	default:
	    //nothing for now
	    break;
    }
}

void QMyo::emgDataServiceCharacteristicChanged(QLowEnergyCharacteristic characteristic, QByteArray data)
{
    QMyo::EmgData emgData;
    
    if(characteristic.uuid() == generateUuid(EmgData0Characteristic))
    {
	for(int i = 0; i < 8; i++)
	{
	    emgData.samples1[i] = data.at(i);
	    emgData.samples2[i] = data.at(i+8);
	}
    }
    else if(characteristic.uuid() == generateUuid(EmgData1Characteristic))
    {
	for(int i = 0; i < 8; i++)
	{
	    emgData.samples1[i] = data.at(i);
	    emgData.samples2[i] = data.at(i+8);
	}
    }
    else if(characteristic.uuid() == generateUuid(EmgData2Characteristic))
    {
	for(int i = 0; i < 8; i++)
	{
	    emgData.samples1[i] = data.at(i);
	    emgData.samples2[i] = data.at(i+8);
	}
    }
    else if(characteristic.uuid() == generateUuid(EmgData3Characteristic))
    {
	for(int i = 0; i < 8; i++)
	{
	    emgData.samples1[i] = data.at(i);
	    emgData.samples2[i] = data.at(i+8);
	}
    }
    else
    {
	//nothing for now
    }
    
    emit updateEmgData(emgData);
}


//Classifier Service
void QMyo::createClassifierDataService()
{
    delete classifierDataService;
    classifierDataService = NULL;
    classifierDataService = controller->createServiceObject(generateUuid(ClassifierService));
    if(!classifierDataService) 
    {
	emit serviceStatus("CLASSIFIER_DATA_SERVICE_NOT_FOUND");
        return;
    }

    QObject::connect(classifierDataService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(classifierDataServiceStateChanged(QLowEnergyService::ServiceState)));
    QObject::connect(classifierDataService, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)), this, SLOT(classifierDataServiceCharacteristicChanged(QLowEnergyCharacteristic,QByteArray)));
    QObject::connect(classifierDataService, SIGNAL(error(QLowEnergyService::ServiceError)), this, SLOT(serviceError(QLowEnergyService::ServiceError)));
    classifierDataService->discoverDetails();
}

void QMyo::classifierDataServiceStateChanged(QLowEnergyService::ServiceState state)
{
    switch(state) 
    {
	case QLowEnergyService::ServiceDiscovered:
	{ 
	    classifierMotionEventChar = classifierDataService->characteristic(generateUuid(ClassifierEventCharacteristic));
	    if (!classifierMotionEventChar.isValid()) 
	    {
		emit serviceStatus("MOTION_EVENT_CHARACTERISTIC_NOT_FOUND");
		break;
	   
	    }
	    QLowEnergyDescriptor motionEventIndication = classifierMotionEventChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
	    if (motionEventIndication.isValid()) 
	    {
		classifierDataService->writeDescriptor(motionEventIndication, byteIndication);
	    }
	    emit serviceStatus("CLASSIFIER_SERVICE_CREATED");
	    break;
	}
	default:
	    //nothing for now
	    break;
    }
}

void QMyo::classifierDataServiceCharacteristicChanged(QLowEnergyCharacteristic characteristic, QByteArray data)
{
    printf("classifierDataServiceCharacteristicChanged1\n");
    if(characteristic.uuid() == generateUuid(ClassifierEventCharacteristic))
    {
	printf("classifierDataServiceCharacteristicChanged\n");
    }
}

//Info Myo
myohw_fw_info_t QMyo::readMyoInfo()
{
    myohw_fw_info_t myoInfoData;
    controlService->readCharacteristic(myoInfoChar);
    eventLoop->exec();
    for(int i=0; i<6; i++)
	myoInfoData.serial_number[i] = (uint8_t)(readResult.at(i));
    myoInfoData.unlock_pose = ((uint8_t)(readResult.at(7))<<8)&0xFF00  + (uint8_t)(readResult.at(6));
    myoInfoData.active_classifier_type = (uint8_t)(readResult.at(8));
    myoInfoData.active_classifier_index = (uint8_t)(readResult.at(9));
    myoInfoData.has_custom_classifier = (uint8_t)(readResult.at(10));
    myoInfoData.stream_indicating = (uint8_t)(readResult.at(11));
    myoInfoData.sku = (uint8_t)(readResult.at(12));
    for(int i=0; i<7; i++)
	myoInfoData.reserved[i] = (uint8_t)(readResult.at(13+i));
    return myoInfoData;
}

QString QMyo::readMyoInfoString()
{
    QString text;
    controlService->readCharacteristic(myoInfoChar);
    eventLoop->exec();
    for(int i=0; i<readResult.size(); i++)
    {
	QString temp;
	temp.sprintf("%X ", (uint8_t)(readResult.at(i)));
	text.append(temp);
    }
    return text;
}

myohw_fw_version_t QMyo::readFirmwareVersion()
{
    myohw_fw_version_t firmwareVersion;
    controlService->readCharacteristic(firmwareVersionChar);
    eventLoop->exec();
    firmwareVersion.minor = ((uint8_t)(readResult.at(1))<<8)&0xFF00  + (uint8_t)(readResult.at(0));
    firmwareVersion.major = ((uint8_t)(readResult.at(3))<<8)&0xFF00  + (uint8_t)(readResult.at(2));
    firmwareVersion.patch = ((uint8_t)(readResult.at(5))<<8)&0xFF00  + (uint8_t)(readResult.at(4));
    firmwareVersion.hardware_rev = ((uint8_t)(readResult.at(7))<<8)&0xFF00  + (uint8_t)(readResult.at(6));
    return firmwareVersion;
}

QString QMyo::readFirmwareVersionString()
{
    QString text;
    controlService->readCharacteristic(firmwareVersionChar);
    eventLoop->exec();
    for(int i=0; i<readResult.size(); i++)
    {
	QString temp;
	temp.sprintf("%X ", (uint8_t)(readResult.at(i)));
	text.append(temp);
    }
    return text;
}


//Commands
bool QMyo::writeCommandSetMode(uint8_t emgMode, uint8_t imuMode, uint8_t classifierMode)
{
    QByteArray command;
    uint8_t payload_size = 0x03;
    command.append(myohw_command_set_mode);
    command.append(payload_size); // (3 bytes)
    command.append(emgMode);
    command.append(imuMode);
    command.append(classifierMode);
    controlService->writeCharacteristic(commandChar, command, QLowEnergyService::WriteWithResponse);
    eventLoop->exec();
    return writeResult;
}

bool QMyo::writeCommandVibrate(uint8_t vibrate)
{
    QByteArray command;
    uint8_t payload_size = 0x01;
    command.append(myohw_command_vibrate);
    command.append(payload_size); // (1 byte)
    command.append(vibrate);
    controlService->writeCharacteristic(commandChar, command, QLowEnergyService::WriteWithResponse);
    eventLoop->exec();
    return writeResult;
}

bool QMyo::writeCommandVibrate2(uint16_t duration1, uint8_t strength1, uint16_t duration2, uint8_t strength2, uint16_t duration3, uint8_t strength3, uint16_t duration4, uint8_t strength4, uint16_t duration5, uint8_t strength5, uint16_t duration6, uint8_t strength6)
{
    QByteArray command;
    uint8_t payload_size = 0x18;
    command.append(myohw_command_vibrate2);
    command.append(payload_size); // (18 bytes)
    command.append((uint8_t)(duration1&0xFF));
    command.append((uint8_t)((duration1>>8)&0xFF));
    command.append(strength1);
    command.append((uint8_t)(duration2&0xFF));
    command.append((uint8_t)((duration2>>8)&0xFF));
    command.append(strength2);
    command.append((uint8_t)(duration3&0xFF));
    command.append((uint8_t)((duration3>>8)&0xFF));
    command.append(strength3);
    command.append((uint8_t)(duration4&0xFF));
    command.append((uint8_t)((duration4>>8)&0xFF));
    command.append(strength4);
    command.append((uint8_t)(duration5&0xFF));
    command.append((uint8_t)((duration5>>8)&0xFF));
    command.append(strength5);
    command.append((uint8_t)(duration6&0xFF));
    command.append((uint8_t)((duration6>>8)&0xFF));
    command.append(strength6);
    controlService->writeCharacteristic(commandChar, command, QLowEnergyService::WriteWithResponse);
    eventLoop->exec();
    return writeResult;
}

bool QMyo::writeCommandDeepSleep()
{
    QByteArray command;
    uint8_t payload_size = 0x00;
    command.append(myohw_command_deep_sleep);
    command.append(payload_size); // (0 bytes)
    controlService->writeCharacteristic(commandChar, command, QLowEnergyService::WriteWithResponse);
    eventLoop->exec();
    return writeResult;
}

bool QMyo::writeCommandSetSleepMode(uint8_t sleepMode)
{
    QByteArray command;
    uint8_t payload_size = 0x01;
    command.append(myohw_command_set_sleep_mode);
    command.append(payload_size); // (1 byte)
    command.append(sleepMode);
    controlService->writeCharacteristic(commandChar, command, QLowEnergyService::WriteWithResponse);
    eventLoop->exec();
    return writeResult;
}

bool QMyo::writeCommandUnlock(uint8_t unlockType)
{
    QByteArray command;
    uint8_t payload_size = 0x01;
    command.append(myohw_command_unlock);
    command.append(payload_size); // (1 byte)
    command.append(unlockType);
    controlService->writeCharacteristic(commandChar, command, QLowEnergyService::WriteWithResponse);
    eventLoop->exec();
    return writeResult;
}

bool QMyo::writeCommandUserAction(uint8_t classifierModelType)
{
    QByteArray command;
    uint8_t payload_size = 0x01;
    command.append(myohw_command_user_action);
    command.append(payload_size); // (1 byte)
    command.append(classifierModelType);
    controlService->writeCharacteristic(commandChar, command, QLowEnergyService::WriteWithResponse);
    eventLoop->exec();
    return writeResult;
}