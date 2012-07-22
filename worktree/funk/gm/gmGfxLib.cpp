#include "gmGfxLib.h"

#include "gmThread.h"
#include "gmMachine.h"
#include "gmHelpers.h"

#include <common/Debug.h>
#include <common/Window.h>
#include <gfx/Renderer.h>
#include <gfx/DrawPrimitives.h>
#include <gfx/ObjLoader.h>

#include <gm/gmBind.h>

#include <math/v2.h>

namespace funk
{
struct gmfGfxLib
{
	static int GM_CDECL gmfBeginDefaultShader(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(0);
		Renderer::Get()->BeginDefaultShader();
		return GM_OK;
	}

	static int GM_CDECL gmfEndDefaultShader(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(0);
		Renderer::Get()->EndDefaultShader();
		return GM_OK;
	}

	static int GM_CDECL gmfDrawLine(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(2);

		if ( GM_IS_PARAM_VEC2(0) )
		{
			GM_CHECK_VEC2_PARAM(p0,0);
			GM_CHECK_VEC2_PARAM(p1,1);
			DrawLine( p0, p1 );
		}

		else if ( GM_IS_PARAM_VEC3(0) )
		{
			GM_CHECK_VEC3_PARAM(p0,0);
			GM_CHECK_VEC3_PARAM(p1,1);
			DrawLine( p0, p1 );
		}
		else
		{
			return GM_EXCEPTION;
		}
		
		return GM_OK;
	}

	static int GM_CDECL gmfDrawSpline(gmThread * a_thread)
	{
		//GM_CHECK_NUM_PARAMS(2);

		if ( GM_IS_PARAM_VEC2(0) )
		{
			GM_CHECK_VEC2_PARAM(v0,0);
			GM_CHECK_VEC2_PARAM(tan0,1);
			GM_CHECK_VEC2_PARAM(v1,2);
			GM_CHECK_VEC2_PARAM(tan1,3);
			GM_FLOAT_PARAM(t0, 4, 0.0f);
			GM_FLOAT_PARAM(t1, 5, 1.0f);
			GM_INT_PARAM(numSegs, 6, 32);

			DrawSpline(v0,tan0,v1,tan1,t0,t1,numSegs);
		}

		else if ( GM_IS_PARAM_VEC3(0) )
		{
			GM_CHECK_VEC3_PARAM(v0,0);
			GM_CHECK_VEC3_PARAM(tan0,1);
			GM_CHECK_VEC3_PARAM(v1,2);
			GM_CHECK_VEC3_PARAM(tan1,3);
			GM_FLOAT_PARAM(t0, 4, 0.0f);
			GM_FLOAT_PARAM(t1, 5, 1.0f);
			GM_INT_PARAM(numSegs, 6, 32);

			DrawSpline(v0,tan0,v1,tan1,t0,t1,numSegs);
		}
		else
		{
			return GM_EXCEPTION;
		}

		return GM_OK;
	}

	static int GM_CDECL gmfClearColor(gmThread * a_thread)
	{
		int numParams = a_thread->GetNumParams();

		if ( numParams == 1 )
		{
			GM_CHECK_VEC3_PARAM(color,0);
			glClearColor( color.x, color.y, color.z, 1.0f );
		}
		else if ( numParams == 2 )
		{
			GM_CHECK_VEC3_PARAM(color,0);
			GM_CHECK_FLOAT_OR_INT_PARAM( alpha, 1);
			glClearColor( color.x, color.y, color.z, alpha );
		}
		else if ( numParams == 3 )
		{
			GM_CHECK_FLOAT_OR_INT_PARAM( r, 0 );
			GM_CHECK_FLOAT_OR_INT_PARAM( g, 1 );
			GM_CHECK_FLOAT_OR_INT_PARAM( b, 2 );
			glClearColor( r, g, b, 1.0f );
		}
		else if ( numParams == 4 )
		{
			GM_CHECK_FLOAT_OR_INT_PARAM( r, 0 );
			GM_CHECK_FLOAT_OR_INT_PARAM( g, 1 );
			GM_CHECK_FLOAT_OR_INT_PARAM( b, 2 );
			GM_CHECK_FLOAT_OR_INT_PARAM( a, 3 );
			glClearColor( r, g, b, a );
		}
		else
		{
			return GM_EXCEPTION;
		}

		return GM_OK;
	}

	static int GM_CDECL gmfClearDepth(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(1);

		GM_CHECK_FLOAT_OR_INT_PARAM( depth, 0 );
		glClearDepth( depth );

		return GM_OK;
	}

	static int GM_CDECL gmfClear(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(1);

		GM_CHECK_INT_PARAM( depth, 0 );
		glClear( depth );

		return GM_OK;
	}

	static int GM_CDECL gmfColor(gmThread * a_thread)
	{
		int numParams = a_thread->GetNumParams();

		if ( numParams == 1 )
		{
			GM_CHECK_VEC3_PARAM( color, 0 );
			glColor3f( color.x, color.y, color.z );
		}
		else if ( numParams == 2 )
		{
			GM_CHECK_VEC3_PARAM( color, 0 );
			GM_CHECK_FLOAT_OR_INT_PARAM( alpha, 1);
			glColor4f( color.x, color.y, color.z, alpha );
		}
		else if ( numParams == 3 )
		{
			GM_CHECK_FLOAT_OR_INT_PARAM( r, 0 );
			GM_CHECK_FLOAT_OR_INT_PARAM( g, 1 );
			GM_CHECK_FLOAT_OR_INT_PARAM( b, 2 );
			glColor3f( r, g, b );
		}
		else if ( numParams == 4 )
		{
			GM_CHECK_FLOAT_OR_INT_PARAM( r, 0 );
			GM_CHECK_FLOAT_OR_INT_PARAM( g, 1 );
			GM_CHECK_FLOAT_OR_INT_PARAM( b, 2 );
			GM_CHECK_FLOAT_OR_INT_PARAM( a, 3 );
			glColor4f( r, g, b, a );
		}
		else
		{
			return GM_EXCEPTION;
		}

		return GM_OK;
	}

	static int GM_CDECL gmfBeginShape(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_INT_PARAM( type, 0 );

		if (!( type >= GL_POINTS && type <= GL_POLYGON ) )
		{
			a_thread->GetMachine()->GetLog().LogEntry("Invalid draw primtive type: 0x%X", type );
		}

		glBegin( type );

		return GM_OK;
	}

	static int GM_CDECL gmfEndShape(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(0);
		glEnd();
		return GM_OK;
	}

	static int GM_CDECL gmfPushMatrix(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(0);
		glPushMatrix();
		return GM_OK;
	}

	static int GM_CDECL gmfPopMatrix(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(0);
		glPopMatrix();
		return GM_OK;
	}

	static int GM_CDECL gmfTranslate(gmThread * a_thread)
	{
		int numParams = a_thread->GetNumParams();

		if ( numParams == 1 )
		{
			if ( GM_IS_PARAM_VEC3(0) )
			{
				GM_VEC3_PARAM(pos,0);
				glTranslatef( pos.x, pos.y, pos.z );
			}
			else if ( GM_IS_PARAM_VEC2(0) )
			{
				GM_VEC2_PARAM(pos,0);
				glTranslatef( pos.x, pos.y, 0.0f );
			}
			else
			{
				return GM_EXCEPTION;
			}
		}

		else if ( numParams == 2 )
		{
			GM_CHECK_FLOAT_OR_INT_PARAM( x, 0 );
			GM_CHECK_FLOAT_OR_INT_PARAM( y, 1 );
			glTranslatef( x, y, 1.0f );
		}
		else if ( numParams == 3 )
		{
			GM_CHECK_FLOAT_OR_INT_PARAM( x, 0 );
			GM_CHECK_FLOAT_OR_INT_PARAM( y, 1 );
			GM_CHECK_FLOAT_OR_INT_PARAM( z, 2 );
			glTranslatef( x, y, z );
		}
		else
		{
			return GM_EXCEPTION;
		}

		return GM_OK;
	}

	static int GM_CDECL gmfSetMatrixMode(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_INT_PARAM( matrixMode, 0 );
		glMatrixMode( matrixMode );
		return GM_OK;
	}	

	static int GM_CDECL gmfIdentity(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(0);
		glLoadIdentity();
		return GM_OK;
	}

	static int GM_CDECL gmfRotate(gmThread * a_thread)
	{
		int numParams = a_thread->GetNumParams();

		if ( numParams == 2 )
		{
			GM_CHECK_FLOAT_OR_INT_PARAM( angle, 0 );
			GM_CHECK_VEC3_PARAM(axis, 1);
			
			glRotatef( angle, axis.x, axis.y, axis.z );
		}

		else if ( numParams == 4 )
		{
			GM_CHECK_FLOAT_OR_INT_PARAM( angle, 0 );
			GM_CHECK_FLOAT_OR_INT_PARAM( axisX, 1 );
			GM_CHECK_FLOAT_OR_INT_PARAM( axisY, 2 );
			GM_CHECK_FLOAT_OR_INT_PARAM( axisZ, 3 );
			
			glRotatef( angle, axisX, axisY, axisZ );
		}

		else
		{
			return GM_EXCEPTION;
		}

		return GM_OK;
	}

	static int GM_CDECL gmfScale(gmThread * a_thread)
	{
		int numParams = a_thread->GetNumParams();

		if ( numParams == 1 )
		{
			if (a_thread->Param(0).m_type == GM_INT || a_thread->Param(0).m_type == GM_FLOAT )
			{
				GM_FLOAT_OR_INT_PARAM( scale, 0, 1.0f );
				glScalef( scale, scale, scale );
			}
			else if ( GM_IS_PARAM_VEC3(0) )
			{
				GM_VEC3_PARAM(scale,0);
				glScalef( scale.x, scale.y, scale.z );
			}
			else if ( GM_IS_PARAM_VEC2(0) )
			{
				GM_VEC2_PARAM(scale,0);
				glScalef( scale.x, scale.y, 1.0f );
			}
			else
			{
				return GM_EXCEPTION;
			}
		}

		else if ( numParams == 2 )
		{
			GM_CHECK_FLOAT_OR_INT_PARAM( x, 0 );
			GM_CHECK_FLOAT_OR_INT_PARAM( y, 1 );
			glScalef( x, y, 1.0f );
		}
		else if ( numParams == 3 )
		{
			GM_CHECK_FLOAT_OR_INT_PARAM( x, 0 );
			GM_CHECK_FLOAT_OR_INT_PARAM( y, 1 );
			GM_CHECK_FLOAT_OR_INT_PARAM( z, 2 );
			
			glScalef( x, y, z );
		}
		else
		{
			return GM_EXCEPTION;
		}

		return GM_OK;
	}

	static int GM_CDECL gmfViewport(gmThread * a_thread)
	{
		int numParams = a_thread->GetNumParams();

		int x = 0;
		int y = 0;
		int width = 1;
		int height = 1;

		if ( numParams == 2 )
		{
			GM_CHECK_VEC2_PARAM( pos,	0 );
			GM_CHECK_VEC2_PARAM( dimen, 1 );
			
			x = (int)pos.x;
			y = (int)pos.y;
			width = (int)dimen.x;
			height = (int)dimen.y;
		}
		else if ( numParams == 4 )
		{
			GM_CHECK_FLOAT_OR_INT_PARAM( xPos, 0 );
			GM_CHECK_FLOAT_OR_INT_PARAM( yPos, 1 );
			GM_CHECK_FLOAT_OR_INT_PARAM( w, 2 );
			GM_CHECK_FLOAT_OR_INT_PARAM( h, 3 );

			x = (int)xPos;
			y = (int)yPos;
			width = (int)w;
			height = (int)h;
		}
		else
		{
			return GM_EXCEPTION;
		}

		// make sure it is within the window
		const v2i windowDimen = Window::Get()->Sizei();
		//assert( x >= 0 && y >= 0 && (x+width)<=windowDimen.x && (x+height)<=windowDimen.y );

		glViewport( x, y, width, height );

		return GM_OK;
	}

	static int GM_CDECL gmfScissor(gmThread * a_thread)
	{
		int numParams = a_thread->GetNumParams();

		int x = 0;
		int y = 0;
		int width = 1;
		int height = 1;

		if ( numParams == 2 )
		{
			GM_CHECK_VEC2_PARAM( pos,	0 );
			GM_CHECK_VEC2_PARAM( dimen, 1 );

			x = (int)pos.x;
			y = (int)pos.y;
			width = (int)dimen.x;
			height = (int)dimen.y;
		}
		else if ( numParams == 4 )
		{
			GM_CHECK_FLOAT_OR_INT_PARAM( xPos, 0 );
			GM_CHECK_FLOAT_OR_INT_PARAM( yPos, 1 );
			GM_CHECK_FLOAT_OR_INT_PARAM( w, 2 );
			GM_CHECK_FLOAT_OR_INT_PARAM( h, 3 );

			x = (int)xPos;
			y = (int)yPos;
			width = (int)w;
			height = (int)h;
		}
		else
		{
			return GM_EXCEPTION;
		}

		// make sure it is within the window
		const v2i windowDimen = Window::Get()->Sizei();
		assert( x >= 0 && y >= 0 && (x+width)<=windowDimen.x && (x+height)<=windowDimen.y );

		glScissor( x, y, width, height );

		return GM_OK;
	}

	static int GM_CDECL gmfPresent(gmThread * a_thread)
	{
		Renderer::Get()->Present();
		return GM_OK;
	}

	static int GM_CDECL gmfVertex(gmThread * a_thread)
	{
		int numParams = a_thread->GetNumParams();

		if ( numParams == 1 )
		{
			if ( GM_IS_PARAM_VEC3(0) )
			{
				GM_VEC3_PARAM(pos,0);
				glVertex3f( pos.x, pos.y, pos.z );
			}
			else if ( GM_IS_PARAM_VEC2(0) )
			{
				GM_VEC2_PARAM(pos,0);
				glVertex2f( pos.x, pos.y );
			}
		}
		else if ( numParams == 2 )
		{
			GM_CHECK_FLOAT_OR_INT_PARAM( x, 0 );
			GM_CHECK_FLOAT_OR_INT_PARAM( y, 1 );
			glVertex2f(x,y);
		}
		else if ( numParams == 3 )
		{
			GM_CHECK_FLOAT_OR_INT_PARAM( x, 0 );
			GM_CHECK_FLOAT_OR_INT_PARAM( y, 1 );
			GM_CHECK_FLOAT_OR_INT_PARAM( z, 2 );
			glVertex3f(x,y,z);
		}

		else
		{
			return GM_EXCEPTION;
		}

		return GM_OK;
	}

	static int GM_CDECL gmfTexCoord(gmThread * a_thread)
	{
		int numParams = a_thread->GetNumParams();

		if ( numParams == 1 )
		{
			if ( GM_IS_PARAM_VEC2(0) )
			{
				GM_VEC2_PARAM(uv,0);
				glTexCoord2f( uv.x, uv.y );
			}
			else
			{
				return GM_EXCEPTION;
			}
		}
		else if ( numParams == 2 )
		{
			GM_CHECK_FLOAT_OR_INT_PARAM( u, 0 );
			GM_CHECK_FLOAT_OR_INT_PARAM( v, 0 );

			glTexCoord2f(u,v);
		}
		else
		{
			return GM_EXCEPTION;
		}

		return GM_OK;
	}

	static int GM_CDECL gmfDrawRect(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_VEC2_PARAM( bottomLeft, 0 );
		GM_CHECK_VEC2_PARAM( dimen, 1 );
		
		DrawRect( bottomLeft, dimen );
		return GM_OK;
	}

	static int GM_CDECL gmfDrawRectRounded(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(3);
		GM_CHECK_VEC2_PARAM( bottomLeft, 0 );
		GM_CHECK_VEC2_PARAM( dimen, 1 );
		GM_CHECK_FLOAT_PARAM( cornerRadius, 2 );

		DrawRectRounded( bottomLeft, dimen, cornerRadius );
		return GM_OK;
	}

	static int GM_CDECL gmfDrawRectRoundedWire(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(3);
		GM_CHECK_VEC2_PARAM( bottomLeft, 0 );
		GM_CHECK_VEC2_PARAM( dimen, 1 );
		GM_CHECK_FLOAT_PARAM( cornerRadius, 2 );

		DrawRectRoundedWire( bottomLeft, dimen, cornerRadius );
		return GM_OK;
	}

	static int GM_CDECL gmfDrawRectRoundedTop(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(3);
		GM_CHECK_VEC2_PARAM( bottomLeft, 0 );
		GM_CHECK_VEC2_PARAM( dimen, 1 );
		GM_CHECK_FLOAT_PARAM( cornerRadius, 2 );

		DrawRectRoundedTop( bottomLeft, dimen, cornerRadius );
		return GM_OK;
	}

	static int GM_CDECL gmfDrawRectRoundedTopWire(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(3);
		GM_CHECK_VEC2_PARAM( bottomLeft, 0 );
		GM_CHECK_VEC2_PARAM( dimen, 1 );
		GM_CHECK_FLOAT_PARAM( cornerRadius, 2 );

		DrawRectRoundedTopWire( bottomLeft, dimen, cornerRadius );
		return GM_OK;
	}

	static int GM_CDECL gmfDrawRectRoundedBot(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(3);
		GM_CHECK_VEC2_PARAM( bottomLeft, 0 );
		GM_CHECK_VEC2_PARAM( dimen, 1 );
		GM_CHECK_FLOAT_PARAM( cornerRadius, 2 );

		DrawRectRoundedBot( bottomLeft, dimen, cornerRadius );
		return GM_OK;
	}

	static int GM_CDECL gmfDrawRectRoundedBotWire(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(3);
		GM_CHECK_VEC2_PARAM( bottomLeft, 0 );
		GM_CHECK_VEC2_PARAM( dimen, 1 );
		GM_CHECK_FLOAT_PARAM( cornerRadius, 2 );

		DrawRectRoundedBotWire( bottomLeft, dimen, cornerRadius );
		return GM_OK;
	}

	static int GM_CDECL gmfDrawRectWire(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_VEC2_PARAM( bottomLeft, 0 );
		GM_CHECK_VEC2_PARAM( dimen, 1 );

		DrawRectWire( bottomLeft, dimen );
		return GM_OK;
	}

	static int GM_CDECL gmfDrawRectTexCoords(gmThread * a_thread)
	{
		int numParams = a_thread->GetNumParams();

		if ( numParams == 2 )
		{
			GM_CHECK_VEC2_PARAM( bottomLeft, 0 );
			GM_CHECK_VEC2_PARAM( dimen, 1 );
			DrawRectTexCoords( bottomLeft, dimen );
		}
		else if ( numParams == 4 )
		{
			GM_CHECK_VEC2_PARAM( bottomLeft, 0 );
			GM_CHECK_VEC2_PARAM( dimen, 1 );
			GM_CHECK_VEC2_PARAM( uv0, 2 );
			GM_CHECK_VEC2_PARAM( uv1, 3 );
			DrawRectTexCoords( bottomLeft, dimen, uv0, uv1 );
			
		}
		else
		{
			return GM_EXCEPTION;
		}
		
		return GM_OK;
	}

	static int GM_CDECL gmfDrawCircle(gmThread * a_thread)
	{
		GM_CHECK_VEC2_PARAM( center, 0 );
		GM_CHECK_FLOAT_PARAM( radius, 1);
		GM_INT_PARAM( numSegs, 2, 32 );

		DrawCircle( center, radius, numSegs);
		return GM_OK;
	}

	static int GM_CDECL gmfDrawCircleWire(gmThread * a_thread)
	{
		GM_CHECK_VEC2_PARAM( center, 0 );
		GM_CHECK_FLOAT_PARAM( radius, 1);
		GM_INT_PARAM( numSegs, 2, 32 );

		DrawCircleWire( center, radius, numSegs);
		return GM_OK;
	}

	static int GM_CDECL gmfDrawDonut(gmThread * a_thread)
	{
		GM_CHECK_VEC2_PARAM( center, 0 );
		GM_CHECK_FLOAT_PARAM( radius, 1);
		GM_CHECK_FLOAT_PARAM( thickness, 2);
		GM_INT_PARAM( numSegs, 3, 32 );

		DrawDonut( center, radius, thickness, numSegs);
		return GM_OK;
	}

	static int GM_CDECL gmfDrawArc(gmThread * a_thread)
	{
		GM_CHECK_VEC2_PARAM( center, 0 );
		GM_CHECK_FLOAT_PARAM( radius, 1);
		GM_CHECK_FLOAT_PARAM( startAngle, 2);
		GM_CHECK_FLOAT_PARAM( endAngle, 3);
		GM_INT_PARAM( numSegs, 4, 32 );

		DrawArc( center, radius, startAngle, endAngle, numSegs);

		return GM_OK;
	}

	static int GM_CDECL gmfDrawTriangle(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(3);

		if ( GM_IS_PARAM_VEC2(0) )
		{
			GM_CHECK_VEC2_PARAM(p0,0);
			GM_CHECK_VEC2_PARAM(p1,1);
			GM_CHECK_VEC2_PARAM(p2,2);
			DrawTriangle( p0, p1, p2 );
		}

		else if ( GM_IS_PARAM_VEC3(0) )
		{
			GM_CHECK_VEC3_PARAM(p0,0);
			GM_CHECK_VEC3_PARAM(p1,1);
			GM_CHECK_VEC3_PARAM(p2,2);
			DrawTriangle( p0, p1, p2 );
		}
		else
		{
			return GM_EXCEPTION;
		}

		return GM_OK;
	}

	static int GM_CDECL gmfDrawTriangleWire(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(3);

		if ( GM_IS_PARAM_VEC2(0) )
		{
			GM_CHECK_VEC2_PARAM(p0,0);
			GM_CHECK_VEC2_PARAM(p1,1);
			GM_CHECK_VEC2_PARAM(p2,2);
			DrawTriangleWire( p0, p1, p2 );
		}

		else if ( GM_IS_PARAM_VEC3(0) )
		{
			GM_CHECK_VEC3_PARAM(p0,0);
			GM_CHECK_VEC3_PARAM(p1,1);
			GM_CHECK_VEC3_PARAM(p2,2);
			DrawTriangleWire( p0, p1, p2 );
		}
		else
		{
			return GM_EXCEPTION;
		}

		return GM_OK;
	}

	static int GM_CDECL gmfDrawCube(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_VEC3_PARAM( dimen, 0 );

		DrawCube( dimen );
		return GM_OK;
	}

	static int GM_CDECL gmfDrawCubeWire(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_VEC3_PARAM( dimen, 0 );

		DrawCubeWire( dimen );
		return GM_OK;
	}

	static int GM_CDECL gmfDrawMayaPlane(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_FLOAT_OR_INT_PARAM( numGrids, 0 );

		DrawMayaPlane( (int)numGrids);
		return GM_OK;
	}

	static int GM_CDECL gmfDrawScreenUV(gmThread * a_thread)
	{
		int numParams = a_thread->GetNumParams();

		if ( numParams == 0 )
		{
			DrawScreenUV();
		}
		else if ( numParams == 2 )
		{
			GM_CHECK_VEC2_PARAM( topLeftUV, 0 );
			GM_CHECK_VEC2_PARAM( dimenUV, 1 );
			DrawScreenUV( topLeftUV, dimenUV );
		}
		else
		{
			return GM_EXCEPTION;
		}
		
		return GM_OK;
	}

	static int GM_CDECL gmfSetLineWidth(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_FLOAT_OR_INT_PARAM(width, 0);
		glLineWidth( max(0.01f,width) );
		return GM_OK;
	}

	static int GM_CDECL gmfSetPointSize(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_FLOAT_OR_INT_PARAM(size, 0);
		glPointSize(size);
		return GM_OK;
	}

	static int GM_CDECL gmfEnable(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_INT_PARAM(val, 0);
		glEnable(val);
		return GM_OK;
	}

	static int GM_CDECL gmfDisable(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_INT_PARAM(val, 0);
		glDisable(val);
		return GM_OK;
	}

	static int GM_CDECL gmfSetBlendFunc(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(2);
		GM_CHECK_INT_PARAM(val0, 0);
		GM_CHECK_INT_PARAM(val1, 1);

		assert( val0 >= GL_ZERO && val0 <= GL_SRC_ALPHA_SATURATE
			&& val1 >= GL_ZERO && val1 <= GL_SRC_ALPHA_SATURATE );

		glBlendFunc( val0, val1 );

		return GM_OK;
	}

	static int GM_CDECL gmfSetWireFrame(gmThread * a_thread)
	{
		GM_CHECK_INT_PARAM(bWireFrame, 0);
		GM_INT_PARAM( mode, 1, GL_FRONT_AND_BACK );

		glPolygonMode( mode, bWireFrame ? GL_LINE : GL_FILL );

		return GM_OK;
	}
	
	static int GM_CDECL gmfCullFace(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_INT_PARAM(mode, 0);

		assert(mode == GL_FRONT || mode == GL_BACK || mode == GL_FRONT_AND_BACK);

		glCullFace(mode);

		return GM_OK;
	}

	static int GM_CDECL gmfDepthMask(gmThread * a_thread)
	{
		GM_CHECK_NUM_PARAMS(1);
		GM_CHECK_INT_PARAM(bDepthMask, 0);

		glDepthMask( bDepthMask != 0 );

		return GM_OK;
	}

	static int GM_CDECL gmfLoadObjVbo(gmThread * a_thread)
	{
		GM_CHECK_STRING_PARAM(file, 0);
		GM_INT_PARAM( swizzleZY, 1, 0 );
		GM_INT_PARAM( genNormals, 2, 0 );

		funk::ObjLoader loader;
		loader.LoadFile(file);
		loader.SetSwizzleZY( swizzleZY != 0 );
		loader.SetGenerateNormals( genNormals != 0 );

		StrongHandle<Vbo> vbo = loader.CreateVbo();
		GM_PUSH_USER_HANDLED( Vbo, vbo.Get() );

		return GM_OK;
	}	
};

static gmFunctionEntry s_gmGfxLib[] = 
{
	// Shader
	GM_LIBFUNC_ENTRY(BeginDefaultShader, Gfx)
	GM_LIBFUNC_ENTRY(EndDefaultShader, Gfx)

	// Draw Primitives
	GM_LIBFUNC_ENTRY(DrawLine, Gfx)
	GM_LIBFUNC_ENTRY(DrawSpline, Gfx)
	GM_LIBFUNC_ENTRY(DrawRect, Gfx)
	GM_LIBFUNC_ENTRY(DrawRectWire, Gfx)	
	GM_LIBFUNC_ENTRY(DrawRectTexCoords, Gfx)
	GM_LIBFUNC_ENTRY(DrawRectRounded, Gfx)
	GM_LIBFUNC_ENTRY(DrawRectRoundedWire, Gfx)
	GM_LIBFUNC_ENTRY(DrawRectRoundedTop, Gfx)
	GM_LIBFUNC_ENTRY(DrawRectRoundedTopWire, Gfx)
	GM_LIBFUNC_ENTRY(DrawRectRoundedBot, Gfx)
	GM_LIBFUNC_ENTRY(DrawRectRoundedBotWire, Gfx)
	GM_LIBFUNC_ENTRY(DrawCircle, Gfx)
	GM_LIBFUNC_ENTRY(DrawCircleWire, Gfx)
	GM_LIBFUNC_ENTRY(DrawDonut, Gfx)
	GM_LIBFUNC_ENTRY(DrawArc, Gfx)
	GM_LIBFUNC_ENTRY(DrawTriangle, Gfx)
	GM_LIBFUNC_ENTRY(DrawTriangleWire, Gfx)
	GM_LIBFUNC_ENTRY(DrawCube, Gfx)
	GM_LIBFUNC_ENTRY(DrawCubeWire, Gfx)
	GM_LIBFUNC_ENTRY(DrawScreenUV, Gfx)
	GM_LIBFUNC_ENTRY(DrawMayaPlane, Gfx)

	// OpenGL
	GM_LIBFUNC_ENTRY(Scissor, Gfx)
	GM_LIBFUNC_ENTRY(Viewport, Gfx)
	GM_LIBFUNC_ENTRY(Present, Gfx)
	GM_LIBFUNC_ENTRY(Vertex, Gfx)
	GM_LIBFUNC_ENTRY(TexCoord, Gfx)
	GM_LIBFUNC_ENTRY(BeginShape, Gfx)
	GM_LIBFUNC_ENTRY(EndShape, Gfx)

	// Clear
	GM_LIBFUNC_ENTRY(Clear, Gfx)
	GM_LIBFUNC_ENTRY(ClearDepth, Gfx)
	GM_LIBFUNC_ENTRY(ClearColor, Gfx)

	// Matrix
	GM_LIBFUNC_ENTRY(SetMatrixMode, Gfx)
	GM_LIBFUNC_ENTRY(Identity, Gfx)
	GM_LIBFUNC_ENTRY(Rotate, Gfx)
	GM_LIBFUNC_ENTRY(Translate, Gfx)
	GM_LIBFUNC_ENTRY(Scale, Gfx)
	GM_LIBFUNC_ENTRY(PushMatrix, Gfx)
	GM_LIBFUNC_ENTRY(PopMatrix, Gfx)

	// Render state
	GM_LIBFUNC_ENTRY(SetLineWidth, Gfx)
	GM_LIBFUNC_ENTRY(SetPointSize, Gfx)
	GM_LIBFUNC_ENTRY(SetBlendFunc, Gfx)
	GM_LIBFUNC_ENTRY(SetWireFrame, Gfx)
	GM_LIBFUNC_ENTRY(DepthMask, Gfx)
	GM_LIBFUNC_ENTRY(CullFace, Gfx)
	GM_LIBFUNC_ENTRY(Color, Gfx)
	GM_LIBFUNC_ENTRY(Enable, Gfx)
	GM_LIBFUNC_ENTRY(Disable, Gfx)

	// Models
	GM_LIBFUNC_ENTRY(LoadObjVbo, Gfx)
};

void gmBindGfxLib( gmMachine * a_machine )
{
	a_machine->RegisterLibrary(s_gmGfxLib, sizeof(s_gmGfxLib) / sizeof(s_gmGfxLib[0]), "Gfx" );
}

}