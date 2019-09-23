/*
 * MainWindow.h
 *
 *  Created on: Jun 29, 2019
 *      Author: taran
 */

#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <QtSerialPort>
#include "ui_MainWindow.h"
#include "ui_StateWidget.h"
#include "Monster.h"
#include "Radar.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow();
    virtual ~MainWindow();
private:
    Ui::MainWindow m_widget;
    Ui::StateWidget m_stateWidget[2];
    QLabel m_labelState;
    Monster m_monster;
    Radar m_radar;

private slots:
    void updatePortListSlot();
    void openPortSlot();
    void closePortSlot();
    void updateGuiSlot();
    void updatedStateSlot(const QString &line);
    void setMotion1Slot();
    void setMotion2Slot();
    void setControllerSlot();
    void setAngleSlot();
    void brakeMotion1Slot();
    void brakeMotion2Slot();
};

#endif /* MAINWINDOW_H_ */
