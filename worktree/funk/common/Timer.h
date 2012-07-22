#ifndef _INCLUDE_TIMER_H
#define _INCLUDE_TIMER_H

namespace funk
{
	class Timer
	{
	public:
		Timer();

		void Start();
		float GetTimeSecs() const;
		float GetTimeMs() const;

	private:

		long long m_start;
		long long m_freq;
	};
}

#endif