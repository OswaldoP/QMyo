#include "include/MyoExample.h"


MyoExample::MyoExample()
{   
    centralWidget = new QWidget();
    layout = new QVBoxLayout();
    layoutLoggers = new QHBoxLayout();
    connectMyo = new QPushButton("Connect Myo");
    readMyoInfo = new QPushButton("Read Myo Info");
    logger = new QTextEdit();
    loggerIMU = new QTextEdit();
    loggerEMG = new QTextEdit();
    
    logger->setReadOnly(true);
    loggerIMU->setReadOnly(true);
    loggerEMG->setReadOnly(true);
    
    layoutLoggers->addWidget(logger);
    layoutLoggers->addWidget(loggerIMU);
    layoutLoggers->addWidget(loggerEMG);
    
    layout->addWidget(connectMyo);
    layout->addWidget(readMyoInfo);
    layout->addLayout(layoutLoggers);
    centralWidget->setLayout(layout);
    
    setCentralWidget(centralWidget);
    
    QObject::connect(connectMyo, SIGNAL(clicked()), this, SLOT(connectMyoClicked()));
    QObject::connect(readMyoInfo, SIGNAL(clicked()), this, SLOT(readMyoInfoClicked()));
    
    myo = new QMyo();
    QObject::connect(myo, SIGNAL(myoStatus(QString)), this, SLOT(myoStatus(QString)));
    QObject::connect(myo, SIGNAL(serviceStatus(QString)), this, SLOT(serviceStatus(QString)));
    //Read Myo
    QObject::connect(myo, SIGNAL(updateImuData(QMyo::ImuData)), this, SLOT(imuDataRead(QMyo::ImuData)));
    QObject::connect(myo, SIGNAL(updateEmgData(QMyo::EmgData)), this, SLOT(emgDataRead(QMyo::EmgData)));
}

MyoExample::~MyoExample()
{
    
}

void MyoExample::connectMyoClicked()
{
    myo->connect(QBluetoothAddress("E0:B0:E4:1F:02:CB"));
}

void MyoExample::readMyoInfoClicked()
{
    bool a = myo->writeCommandSetMode(myohw_emg_mode_send_emg, myohw_imu_mode_send_all, myohw_classifier_mode_enabled);
}

void MyoExample::myoStatus(QString status)
{
    logger->append(QString("MyoStatus: %1").arg(status));
    if(status == QString("CONNECTED"))
    {
	myo->createControlService();
	myo->createImuDataService();
	myo->createEmgDataService();
	myo->createClassifierDataService();
    }
}

void MyoExample::serviceStatus(QString status)
{
    logger->append(QString("ServiceStatus: %1").arg(status));
}

void MyoExample::imuDataRead(QMyo::ImuData dataImu)
{
    loggerIMU->clear();
    
    loggerIMU->append(QString("Accelerometer: "));
    for(int i = 0; i < 3; i++)
	loggerIMU->append(QString("%1 ").arg(dataImu.accelerometer[i]));
    
    loggerIMU->append(QString("OrientationEuler: "));
    for(int i = 0; i < 3; i++)
	loggerIMU->append(QString("%1 ").arg(dataImu.orientationEuler[i]));
    
    loggerIMU->append(QString("Gyroscope: "));
    for(int i = 0; i < 3; i++)
	loggerIMU->append(QString("%1 ").arg(dataImu.gyroscope[i]));
}

void MyoExample::emgDataRead(QMyo::EmgData dataEmg)
{
    loggerEMG->clear();
    
    loggerEMG->append(QString("EMG Signals samples1: "));
    QString msg;
    for(int i = 0; i < 8; i++)
    {
	msg.sprintf("%d ", dataEmg.samples1[i]);
	loggerEMG->append(msg);
    }
    
    loggerEMG->append(QString("EMG Signals samples2: "));
    for(int i = 0; i < 8; i++)
    {
	msg.sprintf("%d ", dataEmg.samples2[i]);
	loggerEMG->append(msg);
    }
}
