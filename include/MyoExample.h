#ifndef MyoExample_H
#define MyoExample_H

#include <QWidget>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>

#include "QMyo.h"

class MyoExample : public QMainWindow
{
    Q_OBJECT
public:
    MyoExample();
    virtual ~MyoExample();
    
private:
    QWidget *centralWidget;
    QVBoxLayout *layout;
    QHBoxLayout *layoutLoggers;
    QPushButton *connectMyo;
    QPushButton *readMyoInfo;
    QTextEdit *logger;
    QTextEdit *loggerIMU;
    QTextEdit *loggerEMG;
    QMyo *myo;
    
signals:
    void sendDataPlotter(int channel, double x);
    
public slots:
    void connectMyoClicked();
    void readMyoInfoClicked();
    void myoStatus(QString status);
    void serviceStatus(QString status);
    void imuDataRead(QMyo::ImuData dataImu);
    void emgDataRead(QMyo::EmgData dataEmg);
};

#endif // MyoExample_H
