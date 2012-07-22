#include "Cam3d.h"

#include <gm/gmBind.h>
#include <gl/glew.h>

namespace funk
{
Cam3d::Cam3d() : m_fov( 45.0f ), m_aspect( 1.0f )
{;}

void Cam3d::Begin()
{
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	gluPerspective( m_fov, m_aspect, m_zNearFar.x, m_zNearFar.y );

	CamBase::Begin();
}

void Cam3d::End()
{
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();

	CamBase::End();
}

void Cam3d::SetViewAngle( float fov, float aspectRatio )
{
	m_fov = fov;
	m_aspect = aspectRatio;
}


GM_REG_NAMESPACE(Cam3d)
{
	GM_MEMFUNC_CONSTRUCTOR(Cam3d)
	{
		GM_CHECK_NUM_PARAMS(0);
		Cam3d * pCam = new Cam3d;
		GM_PUSH_USER_HANDLED( Cam3d, pCam );
		return GM_OK;
	}

	GM_MEMFUNC_DECL(GetPos)
	{
		GM_CHECK_NUM_PARAMS(0);

		GM_GET_THIS_PTR(Cam3d, ptr);
		a_thread->PushVec3( ptr->GetPos() );

		return GM_OK;
	}

	GM_MEMFUNC_DECL(SetPos)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_VEC3_PARAM( pos, 0 );

		GM_GET_THIS_PTR(Cam3d, ptr);
		ptr->SetPos(pos);
		return GM_OK;
	}

	GM_MEMFUNC_DECL(SetLookAt)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_VEC3_PARAM( lookAtPos, 0 );

		GM_GET_THIS_PTR(Cam3d, ptr);
		ptr->SetLookAt( lookAtPos );
		return GM_OK;
	}

	GM_MEMFUNC_DECL(SetNearFar)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_VEC2_PARAM( zNearFar, 0 );		

		GM_GET_THIS_PTR(Cam3d, ptr);
		ptr->SetNearFar( zNearFar );
		return GM_OK;
	}

	GM_GEN_MEMFUNC_VOID_VOID( Cam3d, Begin )
	GM_GEN_MEMFUNC_VOID_VOID( Cam3d, End )
	GM_GEN_MEMFUNC_VOID_FLOAT_FLOAT( Cam3d, SetViewAngle )
}

GM_REG_MEM_BEGIN(Cam3d)
GM_REG_MEMFUNC( Cam3d, Begin )
GM_REG_MEMFUNC( Cam3d, End )
GM_REG_MEMFUNC( Cam3d, GetPos )
GM_REG_MEMFUNC( Cam3d, SetPos )
GM_REG_MEMFUNC( Cam3d, SetLookAt )
GM_REG_MEMFUNC( Cam3d, SetNearFar )
GM_REG_MEMFUNC( Cam3d, SetViewAngle )
GM_REG_HANDLED_DESTRUCTORS(Cam3d)
GM_REG_MEM_END()
GM_BIND_DEFINE(Cam3d)

}