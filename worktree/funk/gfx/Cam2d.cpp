#include "Cam2d.h"

#include <gm/gmBind.h>
#include <gl/glew.h>
#include <common/Window.h>

namespace funk
{
void Cam2d::Begin()
{
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	CamBase::Begin();
	glOrtho( m_bottomLeft.x, m_topRight.x, m_bottomLeft.y, m_topRight.y, m_zNearFar.x, m_zNearFar.y );


	// set to modelview by default
	glMatrixMode( GL_MODELVIEW );
}

void Cam2d::InitScreenSpace()
{
	const v2 windowDimen = Window::Get()->Sizef();
	InitScreenSpaceSize(windowDimen);
}

void Cam2d::End()
{
	CamBase::End();

	glMatrixMode( GL_PROJECTION );
	glPopMatrix();	
}

void Cam2d::SetBounds( v2 bottomLeft, v2 topRight )
{
	m_bottomLeft = bottomLeft;
	m_topRight = topRight;
}

void Cam2d::InitScreenSpaceSize( v2 size )
{
	SetNearFar( v2(-150.0f, 150.0f) );
	SetBounds( v2(0.0f), size );
	SetPos( v3(0.0f) );
	SetLookAt( v3( 0.0f, 0.0f, -1.0f ) );
}

GM_REG_NAMESPACE(Cam2d)
{
	GM_MEMFUNC_DECL(CreateCam2d)
	{
		GM_CHECK_NUM_PARAMS(0);
		GM_PUSH_USER_HANDLED( Cam2d, new Cam2d );
		return GM_OK;
	}

	GM_MEMFUNC_DECL(SetBounds)
	{
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_VEC2_PARAM( botLeft, 0 );
		GM_CHECK_VEC2_PARAM( topRight, 1 );

		GM_GET_THIS_PTR(Cam2d, ptr);
		ptr->SetBounds( botLeft, topRight );
		return GM_OK;
	}

	GM_MEMFUNC_DECL(GetPos)
	{
		GM_CHECK_NUM_PARAMS(0);

		GM_GET_THIS_PTR(Cam2d, ptr);
		a_thread->PushVec3( ptr->GetPos() );

		return GM_OK;
	}

	GM_MEMFUNC_DECL(SetPos)
	{
		GM_CHECK_NUM_PARAMS(1);

		GM_GET_THIS_PTR(Cam2d, ptr);

		if ( GM_IS_PARAM_VEC3(0) )
		{
			GM_VEC3_PARAM( pos, 0 );
			ptr->SetPos(pos);
		}
		else if ( GM_IS_PARAM_VEC2(0) )
		{
			GM_VEC2_PARAM( pos, 0 );
			ptr->SetPos( v3(pos.x, pos.y, 0.0f) );
		}
		else
		{
			return GM_EXCEPTION;
		}

		return GM_OK;
	}

	GM_MEMFUNC_DECL(SetLookAt)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_VEC3_PARAM( lookAtPos, 0 );

		GM_GET_THIS_PTR(Cam2d, ptr);
		ptr->SetLookAt( lookAtPos );
		return GM_OK;
	}

	GM_MEMFUNC_DECL(SetNearFar)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_VEC2_PARAM( zNearFar, 0 );		

		GM_GET_THIS_PTR(Cam2d, ptr);
		ptr->SetNearFar( zNearFar );
		return GM_OK;
	}

	GM_GEN_MEMFUNC_VOID_VOID( Cam2d, Begin )
	GM_GEN_MEMFUNC_VOID_VOID( Cam2d, End )
	GM_GEN_MEMFUNC_VOID_VOID( Cam2d, InitScreenSpace )
	GM_GEN_MEMFUNC_VOID_V2( Cam2d, InitScreenSpaceSize )
}

GM_REG_MEM_BEGIN(Cam2d)
GM_REG_MEMFUNC( Cam2d, Begin )
GM_REG_MEMFUNC( Cam2d, End )
GM_REG_MEMFUNC( Cam2d, InitScreenSpace )
GM_REG_MEMFUNC( Cam2d, InitScreenSpaceSize )
GM_REG_MEMFUNC( Cam2d, SetBounds )
GM_REG_MEMFUNC( Cam2d, GetPos )
GM_REG_MEMFUNC( Cam2d, SetPos )
GM_REG_MEMFUNC( Cam2d, SetLookAt )
GM_REG_MEMFUNC( Cam2d, SetNearFar )
GM_REG_HANDLED_DESTRUCTORS(Cam2d)
GM_REG_MEM_END()
GM_BIND_DEFINE(Cam2d)

}