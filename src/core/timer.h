#ifndef TIMER_H
#define TIMER_H


class Timer
{
public:

    Timer() {
        m_time = m_rate = .1f;
    }

    Timer(float rate) {
        m_time = m_rate = rate;
    }

    // Increments timer
    inline void increment(float delta) {
        m_time -= delta;
    }

    // Resets the timer and sets the rate to a new rate
    inline void setAndResetTimer(float newRate) {
        m_time = m_rate = newRate;
//        m_timesRun = 0; should this be reset?
    }

    // Returns whether or not timer is complete, then resets it
    inline bool finishedThenResetTime() {
        if (m_time > 0.0f)
            return false;

        m_time += m_rate;
        m_timesRun++;
        return true;
    }

    inline float getTime() {
        return m_time;
    }

    inline float getRate() {
        return m_rate;
    }

    inline unsigned int getTimesRun() {
        return m_timesRun;
    }

    


private:
    unsigned int m_timesRun = 0;
    float m_time;
    float m_rate;
};

#endif // TIMER_H
