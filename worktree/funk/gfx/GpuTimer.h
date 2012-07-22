#ifndef _INCLUDE_GPU_TIMER_H_
#define _INCLUDE_GPU_TIMER_H_


namespace funk
{
	class GpuTimer
	{
	public:
		GpuTimer();
		~GpuTimer();
		
		void Begin();
		void End();

		// warning: causes a flush in the GPU
		float GetTimeMs() const;
		
	private:

		unsigned int m_gfxQueryBuffers[2][2];
		int m_gfxQueryIndex;
	};
}

#endif