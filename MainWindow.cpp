/*
 * MainWindow.cpp
 *
 *  Created on: Jun 29, 2019
 *      Author: taran
 */

#include "MainWindow.h"

MainWindow::MainWindow() {
    m_widget.setupUi(this);
    m_widget.groupBox_2->layout()->addWidget(&m_radar);
    m_widget.statusbar->addWidget(&m_labelState);
    m_stateWidget[0].setupUi(m_widget.widgetState1);
    m_stateWidget[1].setupUi(m_widget.widgetState2);
    
    updatePortListSlot();
    connect(m_widget.buttonOpen, SIGNAL(clicked()), this, SLOT(openPortSlot()));
    connect(m_widget.buttonClose, SIGNAL(clicked()), this, SLOT(closePortSlot()));
    connect(m_widget.buttonTest, SIGNAL(clicked()), &m_monster, SLOT(testSlot()));
    connect(m_widget.buttonHoming, SIGNAL(clicked()), &m_monster, SLOT(homingSlot()));
    connect(m_stateWidget[0].leKp, SIGNAL(editingFinished()), this, SLOT(setControllerSlot()));
    connect(m_stateWidget[0].leKi, SIGNAL(editingFinished()), this, SLOT(setControllerSlot()));
    connect(m_stateWidget[0].leTarget, SIGNAL(editingFinished()), this, SLOT(setAngleSlot()));
    connect(&m_monster, SIGNAL(updatedState(const QString &)), this, SLOT(updatedStateSlot(const QString &)));

    connect(m_stateWidget[0].lePwm, SIGNAL(editingFinished()), this, SLOT(setMotion1Slot()));
    connect(m_stateWidget[1].lePwm, SIGNAL(editingFinished()), this, SLOT(setMotion2Slot()));
    connect(m_stateWidget[0].buttonMotionBrake, SIGNAL(clicked()), this, SLOT(brakeMotion1Slot()));
    connect(m_stateWidget[1].buttonMotionBrake, SIGNAL(clicked()), this, SLOT(brakeMotion2Slot()));
}

MainWindow::~MainWindow() {

}

void MainWindow::updatePortListSlot() {
    m_widget.comboPorts->clear();

    QList<QSerialPortInfo> list = QSerialPortInfo::availablePorts();
    for (auto const &info : list) {
        m_widget.comboPorts->addItem(info.portName());
    }
}

void MainWindow::openPortSlot() {
    QString name = m_widget.comboPorts->currentText();
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
    QPalette pal = m_widget.frame->palette();
    if (m_monster.isOpen()) {
        pal.setColor(QPalette::Window, Qt::green);
    }
    else {
        pal.setColor(QPalette::Window, Qt::yellow);
    }
    m_widget.frame->setPalette(pal);
}

void MainWindow::updatedStateSlot(const QString &line) {
    m_labelState.setText(line);

    for (uint i = 0; i < 2; i++) {
        m_stateWidget[i].labelCurrentValue->setText(
                QString("%0").arg(m_monster.getCurrentAmp(i), 4, 'f', 1)
            );
        if (!m_stateWidget[i].lePwm->hasFocus()) {
            m_stateWidget[i].lePwm->setText(
                    QString("%0").arg(m_monster.getPwm(i))
                );
        }
        m_stateWidget[i].labelDirectionValue->setText(
                m_monster.getDirectionString(i)
            );
        m_stateWidget[i].labelDiagValue->setText(
                QString("%0").arg(m_monster.getDiag(i))
            );
        m_stateWidget[i].labelEndstopValue->setText(m_monster.isEndstop(i) ? "true" : "false");
    }
    
    m_radar.setSensor(m_monster.getAngle(0), m_monster.getAngle(1), true);
    
}

void MainWindow::closePortSlot() {
    if (m_monster.isOpen()) {
        m_monster.close();
    }
    updateGuiSlot();
}

void MainWindow::setMotion1Slot() {
    bool ok;
    int value = m_stateWidget[0].lePwm->text().toInt(&ok);
    m_monster.setMotion(0, value);
}

void MainWindow::setMotion2Slot() {
    bool ok;
    int value = m_stateWidget[1].lePwm->text().toInt(&ok);
    m_monster.setMotion(1, value);
}

void MainWindow::brakeMotion1Slot() {
    m_monster.setMotion(0, 0);
    m_stateWidget[0].lePwm->setText("0");
}

void MainWindow::brakeMotion2Slot() {
    m_monster.setMotion(1, 0);
    m_stateWidget[1].lePwm->setText("0");
}

void MainWindow::setControllerSlot() {
    bool ok_kp;
    bool ok_ki;
    int kp = m_stateWidget[0].leKp->text().toInt(&ok_kp);
    int ki = m_stateWidget[0].leKi->text().toInt(&ok_ki);
    if (ok_kp && ok_ki) {
        m_monster.setController(kp, ki);
    }
}

void MainWindow::setAngleSlot() {
    bool ok;
    int angle = m_stateWidget[0].leTarget->text().toInt(&ok);
    if (ok) {
        m_monster.setAngle(angle);
    }
}
