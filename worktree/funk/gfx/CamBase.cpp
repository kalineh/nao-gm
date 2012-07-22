#include "CamBase.h"

#include <gl/glew.h>
#include <gl/GLU.h>

namespace funk
{
CamBase::CamBase() 
: m_eyePos( 0.0f, 0.0f, 1.0f ), m_orient( 0.0f, 0.0f, -1.0f ), m_zNearFar( 0.1f, 100.0f )
{

}

void CamBase::Begin()
{
	v3 lookAt = m_eyePos + m_orient;

	glMatrixMode( GL_PROJECTION );
	gluLookAt( m_eyePos.x, m_eyePos.y, m_eyePos.z, lookAt.x, lookAt.y, lookAt.z, 0.0f, 1.0f, 0.0f );
}

void CamBase::End()
{
	glMatrixMode( GL_PROJECTION );
}

void CamBase::SetNearFar( v2 zNearFar )
{
	m_zNearFar = zNearFar;
}
}