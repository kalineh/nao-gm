#include "GpuTimer.h"

#include <assert.h>
#include <common/Debug.h>
#include <gl/glew.h>
#include <memory>

namespace funk
{
GpuTimer::GpuTimer()
{
	memset( m_gfxQueryBuffers, 0, sizeof(m_gfxQueryBuffers) );
	m_gfxQueryIndex = 0;

	glGenQueries(2, m_gfxQueryBuffers[0]);
	glGenQueries(2, m_gfxQueryBuffers[1]);

	// need to query once
	glQueryCounter(m_gfxQueryBuffers[m_gfxQueryIndex^1][0], GL_TIMESTAMP);
	glQueryCounter(m_gfxQueryBuffers[m_gfxQueryIndex^1][1], GL_TIMESTAMP);
}

GpuTimer::~GpuTimer()
{
	glDeleteQueries(2, m_gfxQueryBuffers[0]);
	glDeleteQueries(2, m_gfxQueryBuffers[1]);
}

void GpuTimer::Begin()
{
	glQueryCounter(m_gfxQueryBuffers[m_gfxQueryIndex][0], GL_TIMESTAMP);
}

void GpuTimer::End()
{
	glQueryCounter(m_gfxQueryBuffers[m_gfxQueryIndex][1], GL_TIMESTAMP);
	m_gfxQueryIndex ^= 1;

	//GLint err = glGetError();
	//CHECK( err == GL_NO_ERROR, "GLerror: 0x%X!", err );
}

float GpuTimer::GetTimeMs() const
{
	GLuint64 startTime, stopTime;
	glGetQueryObjectui64v(m_gfxQueryBuffers[m_gfxQueryIndex][0], GL_QUERY_RESULT, &startTime);
	glGetQueryObjectui64v(m_gfxQueryBuffers[m_gfxQueryIndex][1], GL_QUERY_RESULT, &stopTime);

	//GLint err = glGetError();
	//CHECK( err == GL_NO_ERROR, "GLerror: 0x%X!", err );

	return (stopTime-startTime) / 1000000.0f;
}
}