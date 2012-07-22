#include "Timer.h"

#include <windows.h>

namespace funk
{
Timer::Timer()
{
	QueryPerformanceFrequency((LARGE_INTEGER*)&m_freq);
	Start();
}

void Timer::Start()
{
	QueryPerformanceCounter((LARGE_INTEGER*)&m_start);
}

float Timer::GetTimeSecs() const
{
	return GetTimeMs() * 0.001f;
}

float Timer::GetTimeMs() const
{
	const LARGE_INTEGER &start = *((LARGE_INTEGER*)&m_start);
	const LARGE_INTEGER &freq = *((LARGE_INTEGER*)&m_freq);

	LARGE_INTEGER timeStop;
	QueryPerformanceCounter(&timeStop);

	LONGLONG timeDiff = timeStop.QuadPart - start.QuadPart;
	return timeDiff * 1000.0f / freq.QuadPart;
	return 1.0f;
}

}