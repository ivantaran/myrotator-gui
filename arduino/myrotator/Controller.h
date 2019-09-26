#ifndef CONTROLLER_H_
#define CONTROLLER_H_


class Controller {
public:
    typedef enum {
        Default = 0,
        Pid = 1,
        Homing = 2,
        AngleSpeed = 3
    } ControllerMode;

    Controller() {
        setScale(1000);
    }

    virtual ~Controller() {

    }
    
    void setMode(const ControllerMode &mode) {
        m_mode = mode;
    }

    void setKp(long value) {
        m_kp = value;
    }

    void setKi(long value) {
        m_ki = value;
    }

    void setKd(long value) {
        m_kd = value;
    }
    
    void setScale(long value) {
        m_scale = value;
        m_pidValueLimit = 511 * m_scale;
    }
    
    void setTarget(long value) {
        m_target = value;
    }

    inline const ControllerMode &getMode() {
        return m_mode;
    }
    
    inline long getKp() {
        return m_kp;
    }

    inline long getKi() {
        return m_ki;
    }

    inline long getKd() {
        return m_kd;
    }

    void resetPid() {
        m_pidValue = 0;
        m_error[0] = 0;
        m_error[1] = 0;
        m_error[2] = 0;
    }

    long pid(long currentValue) {
        if (m_mode != ControllerMode::Pid) {
            return 0;
        }

        m_error[2] = m_error[1];
        m_error[1] = m_error[0];
        m_error[0] = currentValue - m_target;
        
        m_pidValue += m_kp * (m_error[0] - m_error[1]) 
                + m_ki * m_error[0] 
                + m_kd * (m_error[0] - m_error[1] - m_error[1] + m_error[2]);

        if (m_pidValue > m_pidValueLimit) {
            m_pidValue = m_pidValueLimit;
        }
        else if (m_pidValue < -m_pidValueLimit) {
            m_pidValue = -m_pidValueLimit;
        }

        return m_pidValue / m_scale;
    }

private:
    ControllerMode m_mode = ControllerMode::Default;
    long m_kp = 0;
    long m_ki = 0;
    long m_kd = 0;
    long m_scale;
    long m_pidValueLimit;
    long m_target = 0;
    long m_error[3] = { 0, 0, 0 };
    long m_pidValue = 0;

};

#endif /* CONTROLLER_H_ */
