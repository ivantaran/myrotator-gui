/*
 * MainWindow.cpp
 *
 *  Created on: Jun 29, 2019
 *      Author: taran
 */

#include "MainWindow.h"

MainWindow::MainWindow() {
    widget.setupUi(this);
    widget.groupBox_2->layout()->addWidget(&m_radar);

    stateWidget[0].setupUi(widget.widgetState1);
    stateWidget[1].setupUi(widget.widgetState2);

    updatePortListSlot();
    connect(widget.buttonOpen, SIGNAL(clicked()), this, SLOT(openPortSlot()));
    connect(widget.buttonClose, SIGNAL(clicked()), this, SLOT(closePortSlot()));
    connect(&m_monster, SIGNAL(updatedState(const QString &)), this, SLOT(updatedStateSlot(const QString &)));

    for (uint i = 0; i < 2; i++) {
        connect(stateWidget[i].buttonMotionRight, SIGNAL(clicked()), &m_monster, SLOT(setMotionRight()));
        connect(stateWidget[i].buttonMotionLeft, SIGNAL(clicked()), &m_monster, SLOT(setMotionLeft()));
        connect(stateWidget[i].buttonMotionBrake, SIGNAL(clicked()), &m_monster, SLOT(setMotionBrake()));
        connect(stateWidget[i].buttonPwmIncrease, SIGNAL(clicked()), &m_monster, SLOT(setPwmIncrease()));
        connect(stateWidget[i].buttonPwmDecrease, SIGNAL(clicked()), &m_monster, SLOT(setPwmDecrease()));
    }
}

MainWindow::~MainWindow() {

}

void MainWindow::updatePortListSlot() {
    widget.comboPorts->clear();

    QList<QSerialPortInfo> list = QSerialPortInfo::availablePorts();
    for (auto const &info : list) {
        widget.comboPorts->addItem(info.portName());
    }
}

void MainWindow::openPortSlot() {
    QString name = widget.comboPorts->currentText();
    QSerialPortInfo info(name);

    if (m_monster.isOpen()) {
        return;
    }

    if (info.isValid() && !info.isBusy()) {
        m_monster.setPort(info);
        bool ok = m_monster.open(QIODevice::ReadWrite);
        if (ok) {
        }
        else {
            qWarning() << m_monster.errorString();
        }
    }
    else {
        if (!info.isValid()) {
            qWarning() << info.systemLocation() << "invalid port";
        }
        if (info.isBusy()) {
            qWarning() << info.systemLocation() << "port is busy";
        }
    }

    updateGuiSlot();
}

void MainWindow::updateGuiSlot() {
    QPalette pal = widget.frame->palette();
    if (m_monster.isOpen()) {
        pal.setColor(QPalette::Window, Qt::green);
    }
    else {
        pal.setColor(QPalette::Window, Qt::yellow);
    }
    widget.frame->setPalette(pal);
}

void MainWindow::updatedStateSlot(const QString &line) {
    widget.textMonitor->appendPlainText(line);

    for (uint i = 0; i < 2; i++) {
        stateWidget[i].labelCurrentSensor->setText(
                QString("%0").arg(m_monster.getCurrentSensor(i))
            );
        stateWidget[i].labelPwm->setText(
                QString("%0").arg(m_monster.getPwm(i))
            );
        stateWidget[i].labelMotion->setText(
                QString("%0").arg(m_monster.getMotionState(i))
            );
        stateWidget[i].labelDiag->setText(
                QString("%0").arg(m_monster.getDiag(i))
            );
    }
    
    m_radar.setSensor(m_monster.getAngle(0), m_monster.getAngle(1), true);
    
}

void MainWindow::closePortSlot() {
    if (m_monster.isOpen()) {
        m_monster.close();
    }
    updateGuiSlot();
}
