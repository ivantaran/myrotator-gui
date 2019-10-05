/*
 * Monster.cpp
 *
 *  Created on: Jun 29, 2019
 *      Author: taran
 */

#include <QtSerialPort>
#include <QSerialPortInfo>
#include <QJsonDocument>

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
                    m_angleMin[i] = (qreal)list.at(position++).toInt(&ok) / 4096.0 * M_PI;
                    m_angleMax[i] = (qreal)list.at(position++).toInt(&ok) / 4096.0 * M_PI;
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

void Monster::setModePid(uint index, int kp, int ki, int kd) {
    write(QString("set %1 pid %2,%3,%4\n").arg(index).arg(kp).arg(ki).arg(kd).toUtf8());
}

void Monster::setTarget(uint index, qreal angle) {
    int value = qRound(-4096.0 * angle / M_PI);
    write(QString("set %1 target %2\n").arg(index).arg(value).toUtf8());
}

void Monster::setModeDefault(uint index) {
    write(QString("set %1 default\n").arg(index).toUtf8());
}

void Monster::setModeHoming(uint index) {
    write(QString("set %1 homing\n").arg(index).toUtf8());
}

void Monster::setConfig(uint index, qreal pwmHoming, qreal pwmMin, qreal pwmMax, qreal angleMin, qreal angleMax, qreal tolerance) {
    int pwmHomingInt = qRound(pwmHoming * 2.55);
    int pwmMinInt = qRound(pwmMin * 2.55);
    int pwmMaxInt = qRound(pwmMax * 2.55);
    int angleMinInt = qRound(4096.0 * angleMin / M_PI);
    int angleMaxInt = qRound(4096.0 * angleMax / M_PI);
    int toleranceInt = qRound(4096.0 * tolerance / M_PI);
    write(QString("set %1 config %2,%3,%4,%5,%6,%7\n").arg(index)
        .arg(pwmHomingInt).arg(pwmMinInt).arg(pwmMaxInt).arg(angleMinInt)
        .arg(angleMaxInt).arg(toleranceInt).toUtf8());
}

void Monster::resetError(uint index) {
    write(QString("set %1 reseterr\n").arg(index).toUtf8());
}

void Monster::requestConfigSlot() {
    write(QString("get config\n").toUtf8());
}

void Monster::readConfig(uint index, const QJsonObject &jsonObject) {
    QJsonValue value;
    
    value = jsonObject.value(QString("pwmHoming"));
    qreal pwmHoming = value.toDouble(0.0);
    value = jsonObject.value(QString("pwmMin"));
    qreal pwmMin = value.toDouble(0.0);
    value = jsonObject.value(QString("pwmMax"));
    qreal pwmMax = value.toDouble(0.0);
    value = jsonObject.value(QString("angleMin"));
    qreal angleMin = qDegreesToRadians(value.toDouble(0.0));
    value = jsonObject.value(QString("angleMax"));
    qreal angleMax = qDegreesToRadians(value.toDouble(0.0));
    value = jsonObject.value(QString("tolerance"));
    qreal tolerance = qDegreesToRadians(value.toDouble(0.0));

    setConfig(index, pwmHoming, pwmMin, pwmMax, angleMin, angleMax, tolerance);
}

void Monster::readSettings(const QString &fileName) {
    QString text;
    QJsonValue value;

    QFile file(fileName);
    file.open(QFile::ReadOnly | QFile::Text);
    text = file.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(text.toUtf8());
    QJsonObject jsonObject = jsonDoc.object();
    
    value = jsonObject.value("azm");
    readConfig(0, value.toObject());
    value = jsonObject.value("elv");
    readConfig(1, value.toObject());
}