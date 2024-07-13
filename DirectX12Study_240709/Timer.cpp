#include "pch.h"

#include "Timer.h"

Timer::Timer()
{
}

Timer::~Timer()
{
}

void Timer::Initialize()
{
    ::QueryPerformanceCounter(&m_prevCounter);
    ::QueryPerformanceFrequency(&m_frequency); // 1초에 몇번 카운트 되는지 체크.
}

void Timer::Update()
{
    ::QueryPerformanceCounter(&m_curCounter);

    auto deltaTime = (double)(m_curCounter.QuadPart - m_prevCounter.QuadPart) / m_frequency.QuadPart;
    m_deltaTime    = deltaTime * 1000.0; // s -> ms
    

    static double accTime = 0.0f;
    static double accCnt  = 0.0f;
    accCnt++;
    accTime += deltaTime;

    if (accTime >= 1.0)
    {
        m_fps = accCnt;
        accTime -= 1.0;
        accCnt = 0;
    }

    m_prevCounter = m_curCounter;
}
