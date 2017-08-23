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
