/*
 * MainWindow.cpp
 *
 *  Created on: Jun 29, 2019
 *      Author: taran
 */

#include "MainWindow.h"

MainWindow::MainWindow() {
    m_widget.setupUi(this);
    m_widget.widget_2->layout()->addWidget(&m_radar);
    m_widget.statusbar->addWidget(&m_labelState);
    m_stateWidget[0].setupUi(m_widget.widgetState1);
    m_stateWidget[1].setupUi(m_widget.widgetState2);
    
    m_doubleValidator.setLocale(QLocale::C);
    m_stateWidget[0].leTarget->setValidator(&m_doubleValidator);
    m_stateWidget[1].leTarget->setValidator(&m_doubleValidator);

    updatePortListSlot();
    connect(m_widget.buttonOpen, SIGNAL(clicked()), this, SLOT(openPortSlot()));
    connect(m_widget.buttonClose, SIGNAL(clicked()), this, SLOT(closePortSlot()));
    connect(m_widget.buttonTest, SIGNAL(clicked()), &m_monster, SLOT(testSlot()));
    connect(m_widget.buttonHoming, SIGNAL(clicked()), &m_monster, SLOT(homingSlot()));

    connect(m_stateWidget[0].leKp, SIGNAL(editingFinished()), this, SLOT(setControllerAzmSlot()));
    connect(m_stateWidget[0].leKi, SIGNAL(editingFinished()), this, SLOT(setControllerAzmSlot()));
    connect(m_stateWidget[0].leKd, SIGNAL(editingFinished()), this, SLOT(setControllerAzmSlot()));
    connect(m_stateWidget[0].leTarget, SIGNAL(editingFinished()), this, SLOT(setTargetAzmSlot()));

    connect(m_stateWidget[1].leKp, SIGNAL(editingFinished()), this, SLOT(setControllerElvSlot()));
    connect(m_stateWidget[1].leKi, SIGNAL(editingFinished()), this, SLOT(setControllerElvSlot()));
    connect(m_stateWidget[1].leKd, SIGNAL(editingFinished()), this, SLOT(setControllerElvSlot()));
    connect(m_stateWidget[1].leTarget, SIGNAL(editingFinished()), this, SLOT(setTargetElvSlot()));

    connect(&m_monster, SIGNAL(updatedState(const QString &)), this, SLOT(updatedStateSlot(const QString &)));

    connect(m_stateWidget[0].lePwm, SIGNAL(editingFinished()), this, SLOT(setMotionAzmSlot()));
    connect(m_stateWidget[1].lePwm, SIGNAL(editingFinished()), this, SLOT(setMotionElvSlot()));
    connect(m_stateWidget[0].buttonMotionBrake, SIGNAL(clicked()), this, SLOT(brakeMotionAzmSlot()));
    connect(m_stateWidget[1].buttonMotionBrake, SIGNAL(clicked()), this, SLOT(brakeMotionElvSlot()));
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

void MainWindow::setMotionAzmSlot() {
    bool ok;
    int value = m_stateWidget[0].lePwm->text().toInt(&ok);
    m_monster.setMotion(0, value);
}

void MainWindow::setMotionElvSlot() {
    bool ok;
    int value = m_stateWidget[1].lePwm->text().toInt(&ok);
    m_monster.setMotion(1, value);
}

void MainWindow::brakeMotionAzmSlot() {
    m_monster.setMotion(0, 0);
    m_stateWidget[0].lePwm->setText("0");
}

void MainWindow::brakeMotionElvSlot() {
    m_monster.setMotion(1, 0);
    m_stateWidget[1].lePwm->setText("0");
}

void MainWindow::setController(uint index) {
    bool ok_kp;
    bool ok_ki;
    bool ok_kd;
    bool ok_rate;
    int kp = m_stateWidget[index].leKp->text().toInt(&ok_kp);
    int ki = m_stateWidget[index].leKi->text().toInt(&ok_ki);
    int kd = m_stateWidget[index].leKd->text().toInt(&ok_kd);
    int rate = m_stateWidget[index].leRateMs->text().toInt(&ok_rate);
    if (ok_kp && ok_ki && ok_kd && ok_rate) {
        ki = (ki * rate) / 1000;
        kd = (kd * 1000) / rate;
        m_monster.setController(index, kp, ki, kd);
    }
}

void MainWindow::setTarget(uint index) {
    bool ok;
    double angle = m_stateWidget[index].leTarget->text().toDouble(&ok);

    if (ok) {
        m_monster.setTargetDegrees(index, angle);
    }
}

void MainWindow::setControllerAzmSlot() {
    setController(0);
}

void MainWindow::setControllerElvSlot() {
    setController(1);
}

void MainWindow::setTargetAzmSlot() {
    setTarget(0);
}

void MainWindow::setTargetElvSlot() {
    setTarget(1);
}
