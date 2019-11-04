/*
 * MainWindow.cpp
 *
 *  Created on: Jun 29, 2019
 *      Author: taran
 */

#include "MainWindow.h"

MainWindow::MainWindow() {
    m_widget.setupUi(this);
    
    m_widget.tab0->layout()->addWidget(&m_radar);
    m_widget.statusbar->addWidget(&m_labelState);
    m_stateWidget[0].setupUi(m_widget.tab1);
    m_stateWidget[1].setupUi(m_widget.tab2);
    
    m_doubleValidator.setLocale(QLocale::C);
    
    m_stateWidget[0].lePwm->setValidator(&m_doubleValidator);
    m_stateWidget[1].lePwm->setValidator(&m_doubleValidator);
    m_stateWidget[0].lePwmHoming->setValidator(&m_doubleValidator);
    m_stateWidget[1].lePwmHoming->setValidator(&m_doubleValidator);

    m_stateWidget[0].leTarget->setValidator(&m_doubleValidator);
    m_stateWidget[1].leTarget->setValidator(&m_doubleValidator);

    updatePortListSlot();
    connect(m_widget.buttonOpen, SIGNAL(clicked()), this, SLOT(openPortSlot()));
    connect(m_widget.buttonClose, SIGNAL(clicked()), this, SLOT(closePortSlot()));
    connect(m_widget.buttonGetConfig, SIGNAL(clicked()), &m_monster, SLOT(requestConfigSlot()));
    connect(m_widget.buttonSetConfig, SIGNAL(clicked()), this, SLOT(readSettingsSlot()));

    connect(m_stateWidget[0].buttonHoming, SIGNAL(clicked()), this, SLOT(setModeHomingAzmSlot()));
    connect(m_stateWidget[0].buttonMotionBrake, SIGNAL(clicked()), this, SLOT(brakeMotionAzmSlot()));
    connect(m_stateWidget[0].buttonResetError, SIGNAL(clicked()), this, SLOT(resetErrorAzmSlot()));
    connect(m_stateWidget[0].buttonPid, SIGNAL(clicked()), this, SLOT(setModePidAzmSlot()));

    connect(m_stateWidget[0].leKp, SIGNAL(editingFinished()), this, SLOT(setControllerAzmSlot()));
    connect(m_stateWidget[0].leKi, SIGNAL(editingFinished()), this, SLOT(setControllerAzmSlot()));
    connect(m_stateWidget[0].leKd, SIGNAL(editingFinished()), this, SLOT(setControllerAzmSlot()));
    connect(m_stateWidget[0].leTarget, SIGNAL(editingFinished()), this, SLOT(setTargetAzmSlot()));
    connect(m_stateWidget[0].lePwm, SIGNAL(editingFinished()), this, SLOT(setMotionAzmSlot()));

    connect(m_stateWidget[0].lePwmHoming, SIGNAL(editingFinished()), this, SLOT(setConfigAzmSlot()));
    connect(m_stateWidget[0].lePwmMin, SIGNAL(editingFinished()), this, SLOT(setConfigAzmSlot()));
    connect(m_stateWidget[0].lePwmMax, SIGNAL(editingFinished()), this, SLOT(setConfigAzmSlot()));
    connect(m_stateWidget[0].leAngleMin, SIGNAL(editingFinished()), this, SLOT(setConfigAzmSlot()));
    connect(m_stateWidget[0].leAngleMax, SIGNAL(editingFinished()), this, SLOT(setConfigAzmSlot()));
    connect(m_stateWidget[0].leTolerance, SIGNAL(editingFinished()), this, SLOT(setConfigAzmSlot()));

    connect(m_stateWidget[1].buttonMotionBrake, SIGNAL(clicked()), this, SLOT(brakeMotionElvSlot()));
    connect(m_stateWidget[1].buttonResetError, SIGNAL(clicked()), this, SLOT(resetErrorElvSlot()));
    connect(m_stateWidget[1].buttonHoming, SIGNAL(clicked()), this, SLOT(setModeHomingElvSlot()));
    connect(m_stateWidget[1].buttonPid, SIGNAL(clicked()), this, SLOT(setModePidElvSlot()));

    connect(m_stateWidget[1].leKp, SIGNAL(editingFinished()), this, SLOT(setControllerElvSlot()));
    connect(m_stateWidget[1].leKi, SIGNAL(editingFinished()), this, SLOT(setControllerElvSlot()));
    connect(m_stateWidget[1].leKd, SIGNAL(editingFinished()), this, SLOT(setControllerElvSlot()));
    connect(m_stateWidget[1].leTarget, SIGNAL(editingFinished()), this, SLOT(setTargetElvSlot()));
    connect(m_stateWidget[1].lePwm, SIGNAL(editingFinished()), this, SLOT(setMotionElvSlot()));

    connect(m_stateWidget[1].lePwmHoming, SIGNAL(editingFinished()), this, SLOT(setConfigElvSlot()));
    connect(m_stateWidget[1].lePwmMin, SIGNAL(editingFinished()), this, SLOT(setConfigElvSlot()));
    connect(m_stateWidget[1].lePwmMax, SIGNAL(editingFinished()), this, SLOT(setConfigElvSlot()));
    connect(m_stateWidget[1].leAngleMin, SIGNAL(editingFinished()), this, SLOT(setConfigElvSlot()));
    connect(m_stateWidget[1].leAngleMax, SIGNAL(editingFinished()), this, SLOT(setConfigElvSlot()));
    connect(m_stateWidget[1].leTolerance, SIGNAL(editingFinished()), this, SLOT(setConfigElvSlot()));

    connect(m_widget.editTerminal, SIGNAL(editingFinished()), this, SLOT(sendStringSlot()));

    connect(&m_monster, SIGNAL(updatedState(const QString &)), this, SLOT(updatedStateSlot(const QString &)));

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

    if (m_monster.getIoDevice()->isOpen()) {
        return;
    }

    // if (info.isValid() && !info.isBusy()) {
    //     m_monster.setPort(info);
    //     bool ok = m_monster.open(QIODevice::ReadWrite);
    //     if (ok) {
    //         m_monster.requestConfigSlot();
    //     }
    //     else {
    //         qWarning() << m_monster.errorString();
    //     }
    // }
    // else {
    //     if (!info.isValid()) {
    //         qWarning() << info.systemLocation() << "invalid port";
    //     }
    //     if (info.isBusy()) {
    //         qWarning() << info.systemLocation() << "port is busy";
    //     }
    // }

    updateGuiSlot();
}

void MainWindow::updateGuiSlot() {
    QPalette pal = m_widget.frame->palette();
    if (m_monster.getIoDevice()->isOpen()) {
        pal.setColor(QPalette::Window, Qt::green);
    }
    else {
        pal.setColor(QPalette::Window, Qt::yellow);
    }
    m_widget.frame->setPalette(pal);
}

void MainWindow::updatedStateSlot(const QString &line) {
    m_labelState.setText(line);
    m_widget.textTerminal->appendPlainText(line);

    for (uint i = 0; i < 2; i++) {
        m_stateWidget[i].labelModeValue->setText(m_monster.getStatusString(i));
        m_stateWidget[i].labelErrorValue->setText(m_monster.getErrorString(i));

        m_stateWidget[i].labelCurrentValue->setText(
                QString("%0").arg(m_monster.getCurrentAmp(i), 4, 'f', 1));

        if (!m_stateWidget[i].lePwm->hasFocus()) {
            m_stateWidget[i].lePwm->setText(
                    QString("%0").arg(m_monster.getPwm(i), 0, 'f', 1));
        }

        if (!m_stateWidget[i].lePwmHoming->hasFocus()) {
            m_stateWidget[i].lePwmHoming->setText(
                    QString("%0").arg(m_monster.getPwmHoming(i), 0, 'f', 1));
        }

        if (!m_stateWidget[i].lePwmMin->hasFocus()) {
            m_stateWidget[i].lePwmMin->setText(
                    QString("%0").arg(m_monster.getPwmMin(i), 0, 'f', 1));
        }

        if (!m_stateWidget[i].lePwmMax->hasFocus()) {
            m_stateWidget[i].lePwmMax->setText(
                    QString("%0").arg(m_monster.getPwmMax(i), 0, 'f', 1));
        }

        if (!m_stateWidget[i].leAngleMin->hasFocus()) {
            m_stateWidget[i].leAngleMin->setText(
                    QString("%0").arg(qRadiansToDegrees(m_monster.getAngleMin(i)), 0, 'f', 1));
        }

        if (!m_stateWidget[i].leAngleMax->hasFocus()) {
            m_stateWidget[i].leAngleMax->setText(
                    QString("%0").arg(qRadiansToDegrees(m_monster.getAngleMax(i)), 0, 'f', 1));
        }

        if (!m_stateWidget[i].leTolerance->hasFocus()) {
            m_stateWidget[i].leTolerance->setText(
                    QString("%0").arg(qRadiansToDegrees(m_monster.getTolerance(i)), 0, 'f', 1));
        }

        m_stateWidget[i].labelDirectionValue->setText(
                m_monster.getDirectionString(i));

        m_stateWidget[i].labelDiagValue->setText(
                QString("%0").arg(m_monster.getDiag(i)));
        
        m_stateWidget[i].labelAngleValue->setText(
            QString("%1").arg(qRadiansToDegrees(m_monster.getAngle(i)), 7, 'f', 1));

        m_stateWidget[i].labelEndstopValue->setText(
            m_monster.isEndstop(i) ? "true" : "false");
    }
    
    m_radar.setSensor(m_monster.getAngle(0), m_monster.getAngle(1), true);
    updateGuiSlot();
}

void MainWindow::closePortSlot() {
    if (m_monster.getIoDevice()->isOpen()) {
        m_monster.getIoDevice()->close();
    }
    updateGuiSlot();
}

void MainWindow::setMotionAzmSlot() {
    bool ok;
    qreal value = m_stateWidget[0].lePwm->text().toDouble(&ok);
    m_monster.setMotion(0, value);
}

void MainWindow::setMotionElvSlot() {
    bool ok;
    qreal value = m_stateWidget[1].lePwm->text().toDouble(&ok);
    m_monster.setMotion(1, value);
}

void MainWindow::setConfigAzmSlot() {
    bool ok;

    qreal pwmHoming = m_stateWidget[0].lePwmHoming->text().toDouble(&ok);
    qreal pwmMin = m_stateWidget[0].lePwmMin->text().toDouble(&ok);
    qreal pwmMax = m_stateWidget[0].lePwmMax->text().toDouble(&ok);
    qreal angleMin = qDegreesToRadians(m_stateWidget[0].leAngleMin->text().toDouble(&ok));
    qreal angleMax = qDegreesToRadians(m_stateWidget[0].leAngleMax->text().toDouble(&ok));
    qreal tolerance = qDegreesToRadians(m_stateWidget[0].leTolerance->text().toDouble(&ok));

    int rate = m_stateWidget[0].leRateMs->text().toInt(&ok);
    int kp = m_stateWidget[0].leKp->text().toInt(&ok);
    int ki = m_stateWidget[0].leKi->text().toInt(&ok);
    int kd = m_stateWidget[0].leKd->text().toInt(&ok);

    m_monster.setConfig(0, pwmHoming, pwmMin, pwmMax, angleMin, angleMax, tolerance, rate, kp, ki, kd);
}

void MainWindow::setConfigElvSlot() {
    bool ok;

    qreal pwmHoming = m_stateWidget[1].lePwmHoming->text().toDouble(&ok);
    qreal pwmMin = m_stateWidget[1].lePwmMin->text().toDouble(&ok);
    qreal pwmMax = m_stateWidget[1].lePwmMax->text().toDouble(&ok);
    qreal angleMin = qDegreesToRadians(m_stateWidget[1].leAngleMin->text().toDouble(&ok));
    qreal angleMax = qDegreesToRadians(m_stateWidget[1].leAngleMax->text().toDouble(&ok));
    qreal tolerance = qDegreesToRadians(m_stateWidget[1].leTolerance->text().toDouble(&ok));

    int rate = m_stateWidget[1].leRateMs->text().toInt(&ok);
    int kp = m_stateWidget[1].leKp->text().toInt(&ok);
    int ki = m_stateWidget[1].leKi->text().toInt(&ok);
    int kd = m_stateWidget[1].leKd->text().toInt(&ok);

    m_monster.setConfig(1, pwmHoming, pwmMin, pwmMax, angleMin, angleMax, tolerance, rate, kp, ki, kd);
}

void MainWindow::brakeMotionAzmSlot() {
    m_monster.setModeDefault(0);
}

void MainWindow::brakeMotionElvSlot() {
    m_monster.setModeDefault(1);
}

void MainWindow::resetErrorAzmSlot() {
    m_monster.clearError(0);
}

void MainWindow::resetErrorElvSlot() {
    m_monster.clearError(1);
}

void MainWindow::setModePid(uint index) {
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
        m_monster.setModePid(index, kp, ki, kd);
    }
}

void MainWindow::setTarget(uint index) {
    bool ok;
    double angle = m_stateWidget[index].leTarget->text().toDouble(&ok);

    if (ok) {
        m_monster.setTarget(index, qDegreesToRadians(angle));
    }
}

void MainWindow::setControllerAzmSlot() {
    setModePid(0);
}

void MainWindow::setControllerElvSlot() {
    setModePid(1);
}

void MainWindow::setTargetAzmSlot() {
    setTarget(0);
}

void MainWindow::setTargetElvSlot() {
    setTarget(1);
}

void MainWindow::setModeHomingAzmSlot() {
    m_monster.setModeHoming(0);
}

void MainWindow::setModeHomingElvSlot() {
    m_monster.setModeHoming(1);
}

void MainWindow::setModePidAzmSlot() {
    setModePid(0);
}

void MainWindow::setModePidElvSlot() {
    setModePid(1);
}

void MainWindow::readSettingsSlot() {
    m_monster.readSettings("monster.json");
}

void MainWindow::sendStringSlot() {
    m_monster.write(m_widget.editTerminal->text() + '\n');
}
