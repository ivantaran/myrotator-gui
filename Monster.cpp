/*
 * Monster.cpp
 *
 *  Created on: Jun 29, 2019
 *      Author: taran
 */

#include <QtSerialPort>
#include <QSerialPortInfo>

#include "Monster.h"

Monster::Monster() {
    setBaudRate(QSerialPort::Baud115200);
    connect(this, SIGNAL(readyRead()), this, SLOT(readyReadSlot()));
}

Monster::~Monster() {

}

void Monster::readyReadSlot() {
    bool ok;
    while (canReadLine()) {
        m_stateLine = QString::fromUtf8(readLine()).trimmed();
        emit updatedState(m_stateLine);
        if (m_stateLine.contains("state:")) {
            m_stateLine = m_stateLine.remove("state:");
            QStringList list = m_stateLine.split(QChar(','));
            if (list.count() >= 14) {
                uint position = 0;
                for (size_t i = 0; i < 2; i++) {
                    m_mode[i] = static_cast<ControllerMode>(list.at(position++).toUInt(&ok));
                    m_error[i] = static_cast<ControllerError>(list.at(position++).toUInt(&ok));
                    m_currentSensor[i] = list.at(position++).toUInt(&ok);
                    m_diag[i] = list.at(position++).toUInt(&ok);
                    m_pwm[i] = list.at(position++).toUInt(&ok);
                    uint ina = list.at(position++).toUInt(&ok) & 0x01;
                    uint inb = list.at(position++).toUInt(&ok) & 0x01;
                    m_direction[i] = ina | (inb << 1);
                    m_angle[i] = (qreal)list.at(position++).toInt(&ok) / -4096.0 * M_PI;
                    m_endstop[i] = list.at(position++).toUInt(&ok) > 0;
                }
            }
        }
    }
}

uint Monster::getPwm(uint index) {
    return index < 2 ? m_pwm[index] : 0;
}

uint Monster::getCurrentAdc(uint index) {
    return index < 2 ? m_currentSensor[index] : 0;
}

qreal Monster::getCurrentAmp(uint index) {
    qreal voltage = 5.0 / 1024.0 * (qreal)getCurrentAdc(index);
    qreal current = voltage / 1500.0;
    current *= 4700.0 / 700.0 * 1500.0;
    return current;
}

uint Monster::getDiag(uint index) {
    return index < 2 ? m_diag[index] : 0;
}

uint Monster::getDirection(uint index) {
    return index < 2 ? m_direction[index] : 0;
}

const QString Monster::getDirectionString(uint index) {
    QString result;
    switch (getDirection(index)) {
    case 0:
        result = "Brakedown";
        break;
    case 1:
        result = "Positive";
        break;
    case 2:
        result = "Negative";
        break;
    case 3:
        result = "Breakup";
        break;
    default:
        result = "Unknown";
        break;
    }
    return result;
}

const QString Monster::getModeString(uint index) {
    QString result;

    if (index < 2) {
        switch (m_mode[index]) {
        case ControllerMode::Default:
            result = "Default";
            break;
        case ControllerMode::Pid:
            result = "PID";
            break;
        case ControllerMode::Homing:
            result = "Homing";
            break;
        case ControllerMode::AngleSpeed:
            result = "Speed";
            break;
        default:
            result = "Unknown";
            break;
        }
    }

    return result;
}

const QString Monster::getErrorString(uint index) {
    QString result;

    if (index < 2) {
        switch (m_error[index]) {
        case ControllerError::Ok:
            result = "No";
            break;
        case ControllerError::ErrorSensor:
            result = "Sensor";
            break;
        case ControllerError::ErrorHoming:
            result = "Homing";
            break;
        default:
            result = "Unknown";
            break;
        }
    }

    return result;
}

qreal Monster::getAngleRadians(uint index) {
    return index < 2 ? m_angle[index] : 0.0;
}

qreal Monster::getAngleDegrees(uint index) {
    return qRadiansToDegrees(getAngleRadians(index));
}

bool Monster::isEndstop(uint index) {
    return index < 2 ? m_endstop[index] : true;
}

void Monster::setMotion(uint index, qreal value) {
    int pwm = qRound(value * 2.55);
    write(QString("set motion%1 %2\n").arg(index + 1).arg(pwm).toUtf8());
}

void Monster::setController(uint index, int kp, int ki, int kd) {
    qWarning() << QString("set ctrl%1_kp %2\n").arg(index + 1).arg(kp).toUtf8() << "\n";

    write(QString("set ctrl%1_kp %2\n").arg(index + 1).arg(kp).toUtf8());
    write(QString("set ctrl%1_ki %2\n").arg(index + 1).arg(ki).toUtf8());
    write(QString("set ctrl%1_kd %2\n").arg(index + 1).arg(kd).toUtf8());
}

void Monster::setTargetLinear(uint index, int angle) {
    write(QString("set ctrl%1_target %2\n").arg(index + 1).arg(angle).toUtf8());
}

void Monster::setTargetRadians(uint index, qreal angle) {
    int value = qRound(-4096.0 * angle / M_PI);
    setTargetLinear(index, value);
}

void Monster::setTargetDegrees(uint index, qreal angle) {
    int value = qRound(-4096.0 * angle / 180.0);
    setTargetLinear(index, value);
}

void Monster::setModePid(uint index) {
    write(QString("set pid%1\n").arg(index + 1).toUtf8());
}

void Monster::setModeHoming(uint index) {
    write(QString("set homing%1\n").arg(index + 1).toUtf8());
}

void Monster::setPwmHoming(uint index, qreal value) {
    int pwm = qRound(value * 2.55);
    write(QString("set pwm_homing%1 %2\n").arg(index + 1).arg(pwm).toUtf8());
}

void Monster::resetError(uint index) {
    qWarning() << QString("set reset_error%1\n").arg(index + 1);
    write(QString("set reset_error%1\n").arg(index + 1).toUtf8());
}
