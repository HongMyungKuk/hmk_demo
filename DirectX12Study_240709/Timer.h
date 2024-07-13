#pragma once

class Timer
{
  public:
    Timer();
    ~Timer();

    void Initialize();
    void Update();

  public:
    double GetDT()
    {
        return m_deltaTime;
    }
    double GetFPS()
    {
        return m_fps;
    }

  private:
    LARGE_INTEGER m_prevCounter = {};
    LARGE_INTEGER m_curCounter  = {};
    LARGE_INTEGER m_frequency   = {};

    double m_deltaTime = 0.0;
    double m_fps       = 0.0f;
};
