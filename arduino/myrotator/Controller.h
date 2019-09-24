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
    
    inline const ControllerMode &getMode() {
        return m_mode;
    }

private:
    ControllerMode m_mode = Controller::Default;
    long m_kp = 30;
    long m_ki = 1000;
    long m_kd = 1000;
    long m_scale = 1000;
};

#endif /* CONTROLLER_H_ */
