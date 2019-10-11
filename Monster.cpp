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
    m_timerSlowId = startTimer(3000);
    m_timerFastId = startTimer(200);
}

Monster::~Monster() {

}

void Monster::timerEvent(QTimerEvent *event) {
    if (event->timerId() == m_timerSlowId) {
        requestConfigSlot();
    }
    else if (event->timerId() == m_timerFastId) {
        requestPosition();
    }
}

void Monster::acceptConfigRegister(uint index, uint addr, int value) {
    if (index > 1) {
        return;
    }

    switch (addr) {
    case 0:
        m_pwmHoming[index] = (qreal)value / 2.55;
        break;
    case 1:
        m_pwmMin[index] = (qreal)value / 2.55;
        break;
    case 2:
        m_pwmMax[index] = (qreal)value / 2.55;
        break;
    case 3:
        m_angleMin[index] = (qreal)value / 4096.0 * M_PI;
        break;
    case 4:
        m_angleMax[index] = (qreal)value / 4096.0 * M_PI;
        break;
    case 5:
        m_tolerance[index] = (qreal)value / 4096.0 * M_PI;
        break;
    }
}

void Monster::readyReadSlot() {
    bool ok_addr, ok_value;
    uint addr;
    int vi;
    qreal vr;

    while (canReadLine()) {
        m_stateLine = QString::fromUtf8(readLine()).trimmed();

        QStringList list = m_stateLine.split(QChar(' '));
        QRegExp r("([A-Z]{2})([0-9]+)?(\\,)?([-+]?[0-9]*\\.?[0-9]*)");
        for (const auto &sub : list) {
            r.exactMatch(sub);
            if (r.cap(1) == "AR") {
                addr = r.cap(2).toUInt(&ok_addr);
                vi = r.cap(4).toInt(&ok_value);
                if (!ok_addr || !ok_value) {
                    continue;
                }
                acceptConfigRegister(0, addr, vi);
            }
            else if (r.cap(1) == "ER") {
                addr = r.cap(2).toUInt(&ok_addr);
                vi = r.cap(4).toInt(&ok_value);
                if (!ok_addr || !ok_value) {
                    continue;
                }
                acceptConfigRegister(1, addr, vi);
            }
            else if (r.cap(1) == "GS") {
                vi = r.cap(2).toInt(&ok_value);
                m_status[0] = vi & 0xff;
                m_status[1] = (vi >> 8) & 0xff;
            }
            else if (r.cap(1) == "GE") {
                vi = r.cap(2).toInt(&ok_value);
                m_error[0] = vi & 0xff;
                m_error[1] = (vi >> 8) & 0xff;
            }
            else if (r.cap(1) == "AZ") {
                vr = r.cap(2).toDouble(&ok_value);
                if (ok_value) {
                    m_angle[0] = qDegreesToRadians(vr);
                }
            }
            else if (r.cap(1) == "EL") {
                vr = r.cap(2).toDouble(&ok_value);
                if (ok_value) {
                    m_angle[1] = qDegreesToRadians(vr);
                }
            }

        }

        emit updatedState(m_stateLine);
    //     if (m_stateLine.contains("state:")) {
    //         m_stateLine = m_stateLine.remove("state:");
    //         QStringList list = m_stateLine.split(QChar(','));
    //         if (list.count() >= 14) {
    //             uint position = 0;
    //             for (size_t i = 0; i < 2; i++) {
    //                 m_mode[i] = static_cast<ControllerMode>(list.at(position++).toUInt(&ok));
    //                 m_error[i] = static_cast<ControllerError>(list.at(position++).toUInt(&ok));
    //                 m_currentSensor[i] = list.at(position++).toUInt(&ok);
    //                 m_diag[i] = list.at(position++).toUInt(&ok);
    //                 m_pwm[i] = (qreal)list.at(position++).toUInt(&ok) / 2.55;
    //                 uint ina = list.at(position++).toUInt(&ok) & 0x01;
    //                 uint inb = list.at(position++).toUInt(&ok) & 0x01;
    //                 m_direction[i] = ina | (inb << 1);
    //                 m_angle[i] = (qreal)list.at(position++).toInt(&ok) / -4096.0 * M_PI;
    //                 m_endstop[i] = list.at(position++).toUInt(&ok) > 0;
    //             }
    //         }
    //     }
    //     else if (m_stateLine.contains("config:")) {
    //         m_stateLine = m_stateLine.remove("config:");
    //         QStringList list = m_stateLine.split(QChar(','));
    //         if (list.count() >= 12) {
    //             uint position = 0;
    //             for (size_t i = 0; i < 2; i++) {
    //                 m_pwmHoming[i] = (qreal)list.at(position++).toInt(&ok) / 2.55;
    //                 m_pwmMin[i] = (qreal)list.at(position++).toInt(&ok) / 2.55;
    //                 m_pwmMax[i] = (qreal)list.at(position++).toInt(&ok) / 2.55;
    //                 m_angleMin[i] = (qreal)list.at(position++).toInt(&ok) / 4096.0 * M_PI;
    //                 m_angleMax[i] = (qreal)list.at(position++).toInt(&ok) / 4096.0 * M_PI;
    //                 m_tolerance[i] = (qreal)list.at(position++).toInt(&ok) / 4096.0 * M_PI;
    //             }
    //         }
    //     }
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

const QString Monster::getStatusString(uint index) {
    QString result;

    if (index < 2) {
        switch (m_status[index]) {
        case StatusIdle:
            result = "Idle";
            break;
        case StatusMoving:
            result = "Moving";
            break;
        case StatusPointing:
            result = "Pointing";
            break;
        case StatusHoming:
            result = "Homing";
            break;
        case StatusUnhoming:
            result = "Unhoming";
            break;
        case StatusError:
            result = "Error";
            break;
        default:
            result = QString("Unknown [0x%1]").arg(m_status[index], 2, 16, QChar('0'));
            break;
        }
    }

    return result;
}

const QString Monster::getErrorString(uint index) {
    QString result;

    if (index < 2) {
        result = QString("%1").arg(m_error[index], 8, 2, QChar('0'));
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

void Monster::clearError(uint index) {
    write(QString("%1 \n").arg(index == 0 ? "AC" : "EC").toUtf8());
}

void Monster::requestConfigSlot() {
    if (isOpen()) {
        write(QString("AR0 AR1 AR2 AR3 AR4 AR5 AR6 AR7 AR8 \n").toUtf8());
        write(QString("ER0 ER1 ER2 ER3 ER4 ER5 ER6 ER7 ER8 \n").toUtf8());
        write(QString("GE GS \n").toUtf8());
    }
}

void Monster::requestPosition() {
    if (isOpen()) {
        write(QString("AZ EL \n").toUtf8());
    }
}

void Monster::setPwmHoming(uint index, qreal value) {
    write(QString("%1%2,%3 \n")
        .arg(index == 0 ? "AW" : "EW")
        .arg('0')
        .arg(qRound(value * 2.55)).toUtf8());
}

void Monster::setPwmMin(uint index, qreal value) {
    write(QString("%1%2,%3 \n")
        .arg(index == 0 ? "AW" : "EW")
        .arg('1')
        .arg(qRound(value * 2.55)).toUtf8());
}

void Monster::setPwmMax(uint index, qreal value) {
    write(QString("%1%2,%3 \n")
        .arg(index == 0 ? "AW" : "EW")
        .arg('2')
        .arg(qRound(value * 2.55)).toUtf8());
}

void Monster::setAngleMin(uint index, qreal value) {
    write(QString("%1%2,%3 \n")
        .arg(index == 0 ? "AW" : "EW")
        .arg('3')
        .arg(qRound(4096.0 * value / M_PI)).toUtf8());
}

void Monster::setAngleMax(uint index, qreal value) {
    write(QString("%1%2,%3 \n")
        .arg(index == 0 ? "AW" : "EW")
        .arg('4')
        .arg(qRound(4096.0 * value / M_PI)).toUtf8());
}

void Monster::setTolerance(uint index, qreal value) {
    write(QString("%1%2,%3 \n")
        .arg(index == 0 ? "AW" : "EW")
        .arg('5')
        .arg(qRound(4096.0 * value / M_PI)).toUtf8());
}

void Monster::setKp(uint index, int value) {
    write(QString("%1%2,%3 \n")
        .arg(index == 0 ? "AW" : "EW")
        .arg('6')
        .arg(value).toUtf8());
}

void Monster::setKi(uint index, int value) {
    write(QString("%1%2,%3 \n")
        .arg(index == 0 ? "AW" : "EW")
        .arg('7')
        .arg(value).toUtf8());
}

void Monster::setKd(uint index, int value) {
    write(QString("%1%2,%3 \n")
        .arg(index == 0 ? "AW" : "EW")
        .arg('8')
        .arg(value).toUtf8());
}

void Monster::setConfig(uint index, qreal pwmHoming, qreal pwmMin, qreal pwmMax, qreal angleMin, qreal angleMax, qreal tolerance, 
        int rate, int kp, int ki, int kd) {

    ki = (ki * rate) / 1000;
    kd = (kd * 1000) / rate;

    setPwmHoming(index, pwmHoming);
    setPwmMin(index, pwmMin);
    setPwmMax(index, pwmMax);
    setAngleMin(index, angleMin);
    setAngleMax(index, angleMax);
    setTolerance(index, tolerance);
    setKp(index, kp);
    setKi(index, ki);
    setKd(index, kd);
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

    value = jsonObject.value(QString("rate"));
    int rate = value.toInt(100);
    value = jsonObject.value(QString("kp"));
    int kp = value.toInt(0);
    value = jsonObject.value(QString("ki"));
    int ki = value.toInt(0);
    value = jsonObject.value(QString("kd"));
    int kd = value.toInt(0);

    setConfig(index, pwmHoming, pwmMin, pwmMax, angleMin, angleMax, tolerance, rate, kp, ki, kd);
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
    clearError(0);
    value = jsonObject.value("elv");
    readConfig(1, value.toObject());
    clearError(1);
}