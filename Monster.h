/*
 * Monster.h
 *
 *  Created on: Jun 29, 2019
 *      Author: taran
 */

#ifndef MONSTER_H_
#define MONSTER_H_

#include <QtSerialPort>

class Monster : public QSerialPort {
    Q_OBJECT
public:
    Monster();
    virtual ~Monster();
    uint getPwm(uint index);
    uint getCurrentAdc(uint index);
    qreal getCurrentAmp(uint index);
    uint getDiag(uint index);
    uint getDirection(uint index);
    const QString getDirectionString(uint index);
    qreal getAngle(uint index);
    bool isEndstop(uint index);
    void setMotion(uint index, int value);
    void setController(int kp, int ki);
    void setAngle(int angle);

private:
    QString m_stateLine;
    uint m_pwm[2] = { 0, 0 };
    uint m_currentSensor[2] = { 0, 0 };
    uint m_diag[2] = { 0, 0 };
    uint m_direction[2] = { 0, 0 };
    qreal m_angle[2] = { 0.0, 0.0 };
    bool m_endstop[2] = { true, true };

private slots:
    void readyReadSlot();

public slots:
    void testSlot();
    void homingSlot();

signals:
    void updatedState(const QString &line);
};

#endif /* MONSTER_H_ */
