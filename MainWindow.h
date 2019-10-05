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
    QDoubleValidator m_doubleValidator;

    void setModePid(uint index);
    void setTarget(uint index);

private slots:
    void updatePortListSlot();
    void openPortSlot();
    void closePortSlot();
    void updateGuiSlot();
    void updatedStateSlot(const QString &line);
    void setMotionAzmSlot();
    void setMotionElvSlot();
    void setControllerAzmSlot();
    void setControllerElvSlot();
    void setTargetAzmSlot();
    void setTargetElvSlot();
    void brakeMotionAzmSlot();
    void brakeMotionElvSlot();
    void resetErrorAzmSlot();
    void resetErrorElvSlot();
    void setModeHomingAzmSlot();
    void setModeHomingElvSlot();
    void setModePidAzmSlot();
    void setModePidElvSlot();
    void setConfigAzmSlot();
    void setConfigElvSlot();
    void readSettingsSlot();
};

#endif /* MAINWINDOW_H_ */
