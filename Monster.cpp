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
                m_currentSensor[0] = list.at(0).toUInt(&ok);
                m_currentSensor[1] = list.at(1).toUInt(&ok);
                m_diag[0] = list.at(2).toUInt(&ok);
                m_diag[1] = list.at(3).toUInt(&ok);
                m_pwm[0] = list.at(4).toUInt(&ok);
                m_pwm[1] = list.at(5).toUInt(&ok);
                uint ina1 = list.at(6).toUInt(&ok) & 0x01;
                uint inb1 = list.at(7).toUInt(&ok) & 0x01;
                uint ina2 = list.at(8).toUInt(&ok) & 0x01;
                uint inb2 = list.at(9).toUInt(&ok) & 0x01;
                m_direction[0] = ina1 | (inb1 << 1);
                m_direction[1] = ina2 | (inb2 << 1);
                m_angle[0] = (qreal)list.at(10).toInt(&ok) / -4096.0 * M_PI;
                m_angle[1] = (qreal)list.at(11).toInt(&ok) / -4096.0 * M_PI;
                m_endstop[0] = list.at(12).toUInt(&ok) > 0;
                m_endstop[1] = list.at(13).toUInt(&ok) > 0;
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
        result = "brakedown";
        break;
    case 1:
        result = "positive";
        break;
    case 2:
        result = "negative";
        break;
    case 3:
        result = "breakup";
        break;
    default:
        result = "unknown";
        break;
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
