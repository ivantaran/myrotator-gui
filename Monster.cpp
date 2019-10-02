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
                    m_pwm[i] = (qreal)list.at(position++).toUInt(&ok) / 2.55;
                    uint ina = list.at(position++).toUInt(&ok) & 0x01;
                    uint inb = list.at(position++).toUInt(&ok) & 0x01;
                    m_direction[i] = ina | (inb << 1);
                    m_angle[i] = (qreal)list.at(position++).toInt(&ok) / -4096.0 * M_PI;
                    m_endstop[i] = list.at(position++).toUInt(&ok) > 0;
                }
            }
        }
        else if (m_stateLine.contains("config:")) {
            m_stateLine = m_stateLine.remove("config:");
            QStringList list = m_stateLine.split(QChar(','));
            if (list.count() >= 12) {
                uint position = 0;
                for (size_t i = 0; i < 2; i++) {
                    m_pwmHoming[i] = (qreal)list.at(position++).toInt(&ok) / 2.55;
                    m_pwmMin[i] = (qreal)list.at(position++).toInt(&ok) / 2.55;
                    m_pwmMax[i] = (qreal)list.at(position++).toInt(&ok) / 2.55;
                    m_angleMin[i] = (qreal)list.at(position++).toInt(&ok) / -4096.0 * M_PI;
                    m_angleMax[i] = (qreal)list.at(position++).toInt(&ok) / -4096.0 * M_PI;
                    m_tolerance[i] = (qreal)list.at(position++).toInt(&ok) / 4096.0 * M_PI;
                }
            }
        }
    }
}

qreal Monster::getPwm(uint index) {
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
        case ModeDefault:
            result = "Default";
            break;
        case ModePid:
            result = "PID";
            break;
        case ModeHoming:
            result = "Homing";
            break;
        case ModeSpeed:
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
        case ErrorOk:
            result = "No";
            break;
        case ErrorSensor:
            result = "Sensor";
            break;
        case ErrorHoming:
            result = "Homing";
            break;
        default:
            result = "Unknown";
            break;
        }
    }

    return result;
}

qreal Monster::getAngle(uint index) {
    return index < 2 ? m_angle[index] : 0.0;
}

bool Monster::isEndstop(uint index) {
    return index < 2 ? m_endstop[index] : true;
}

qreal Monster::getPwmHoming(uint index) {
    return index < 2 ? m_pwmHoming[index] : 0.0;
}

qreal Monster::getPwmMin(uint index) {
    return index < 2 ? m_pwmMin[index] : 0.0;
}

qreal Monster::getPwmMax(uint index) {
    return index < 2 ? m_pwmMax[index] : 0.0;
}

qreal Monster::getAngleMin(uint index) {
    return index < 2 ? m_angleMin[index] : 0.0;
}

qreal Monster::getAngleMax(uint index) {
    return index < 2 ? m_angleMax[index] : 0.0;
}

qreal Monster::getTolerance(uint index) {
    return index < 2 ? m_tolerance[index] : 0.0;
}

void Monster::setMotion(uint index, qreal value) {
    int pwm = qRound(value * 2.55);
    write(QString("set %1 motion %2\n").arg(index).arg(pwm).toUtf8());
}

void Monster::setController(uint index, int kp, int ki, int kd) {
    write(QString("set %1 kp %2\n").arg(index).arg(kp).toUtf8());
    write(QString("set %1 ki %2\n").arg(index).arg(ki).toUtf8());
    write(QString("set %1 kd %2\n").arg(index).arg(kd).toUtf8());
}

void Monster::setTargetLinear(uint index, int angle) {
    write(QString("set %1 target %2\n").arg(index).arg(angle).toUtf8());
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
    write(QString("set %1 pid\n").arg(index).toUtf8());
}

void Monster::setModeHoming(uint index) {
    write(QString("set %1 homing\n").arg(index).toUtf8());
}

void Monster::setPwmHoming(uint index, qreal value) {
    int pwm = qRound(value * 2.55);
    write(QString("set %1 pwm_homing %2\n").arg(index).arg(pwm).toUtf8());
}

void Monster::setPwmMin(uint index, qreal value) {
    int pwm = qRound(value * 2.55);
    write(QString("set %1 pwm_homing %2\n").arg(index).arg(pwm).toUtf8());
}

void Monster::resetError(uint index) {
    write(QString("set %1 reseterr\n").arg(index).toUtf8());
}

void Monster::requestConfigSlot() {
    write(QString("get config\n").toUtf8());
}
