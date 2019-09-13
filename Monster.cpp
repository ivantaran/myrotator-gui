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
            if (list.count() >= 12) {
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
                m_motionState[0] = ina1 | (inb1 << 1);
                m_motionState[1] = ina2 | (inb2 << 1);
                m_angle[0] = (qreal)list.at(10).toUInt(&ok) / 2048.0 * M_PI;
                m_angle[1] = (qreal)list.at(11).toUInt(&ok) / 2048.0 * M_PI;
            }
        }
    }
}

uint Monster::getPwm(uint index) {
    return index < 2 ? m_pwm[index] : 0;
}

uint Monster::getCurrentSensor(uint index) {
    return index < 2 ? m_currentSensor[index] : 0;
}

uint Monster::getDiag(uint index) {
    return index < 2 ? m_diag[index] : 0;
}

uint Monster::getMotionState(uint index) {
    return index < 2 ? m_motionState[index] : 0;
}

qreal Monster::getAngle(uint index) {
    return index < 2 ? m_angle[index] : 0.0;
}

void Monster::setMotion(uint index, int value) {
    write(QString("set motion%1 %2\n").arg(index + 1).arg(value).toUtf8());
}

void Monster::setController(int kp, int ki) {
    write(QString("set ctrl_kp %1\n").arg(kp).toUtf8());
    write(QString("set ctrl_ki %1\n").arg(ki).toUtf8());
}

void Monster::setAngle(int angle) {
    write(QString("set ctrl_angle %1\n").arg(angle).toUtf8());
}

void Monster::testSlot() {
    qWarning() << "testSlot";
    write("set con\n");
}
