#include "DrawPrimitives.h"

#include <math.h>
#include <gl/glew.h>
#include <common/Window.h>
#include <common/Debug.h>

#include "Renderer.h"

namespace funk
{

#define CHECK_SHADER WARN(Renderer::Get()->IsShaderBound(), "Draw primitive: No shader bound!");

void DrawLine( const v2 start, const v2 end )
{
	CHECK_SHADER;

	glBegin( GL_LINES );
	glVertex2f( start.x, start.y );
	glVertex2f( end.x, end.y );
	glEnd();
}

void DrawLine( const v3 start, const v3 end )
{
	CHECK_SHADER;

	glBegin( GL_LINES );
	glVertex3f( start.x, start.y, start.z );
	glVertex3f( end.x, end.y, end.z );
	glEnd();
}

void DrawRect( const v2 bottomLeft, const v2 dimen )
{
	glBegin( GL_QUADS );
	glVertex2f( bottomLeft.x + dimen.x, bottomLeft.y );
	glVertex2f( bottomLeft.x, bottomLeft.y );
	glVertex2f( bottomLeft.x, bottomLeft.y + dimen.y);
	glVertex2f( bottomLeft.x + dimen.x, bottomLeft.y + dimen.y);
	glEnd();
}

void DrawCircle( const v2 center, float radius, int segments )
{
	CHECK_SHADER;

	float deltaAngle = 2.0f * PI / segments;

	glBegin( GL_TRIANGLE_FAN );

	glVertex2f( center.x, center.y );

	for ( int i = 0; i <= segments; ++i )
	{
		float currAngle = deltaAngle * i;
		v2 pos = center + unitCircle(currAngle) * radius;
		glVertex2f( pos.x, pos.y );
	}

	glEnd();
}

void DrawCircleWire( const v2 center, float radius, int segments )
{
	CHECK_SHADER;

	float deltaAngle = 2.0f * PI / segments;

	glBegin( GL_LINE_LOOP );

	for ( int i = 0; i < segments; ++i )
	{
		float currAngle = deltaAngle * i;
		v2 pos = center + unitCircle(currAngle) * radius;
		glVertex2f( pos.x, pos.y );
	}

	glEnd();
}


void DrawDonut( const v2 center, float radius, float thickness, int segments /*= 32 */ )
{
	CHECK_SHADER;

	float deltaAngle = 2.0f * PI / segments;

	glBegin( GL_TRIANGLE_STRIP );

	for ( int i = 0; i <= segments; ++i )
	{
		float currAngle = deltaAngle * i;

		v2 dir = v2(cosf(currAngle), sinf(currAngle) );
		v2 pos0 = center+ (radius-0.5f*thickness)*dir;
		v2 pos1 = center+ (radius+0.5f*thickness)*dir;

		glVertex2f( pos0.x, pos0.y );
		glVertex2f( pos1.x, pos1.y );
	}

	glEnd();
}

void DrawArc( const v2 center, float radius, float startAngle, float endAngle, int segments )
{
	CHECK_SHADER;

	float deltaAngle = (endAngle-startAngle) / segments;

	glBegin( GL_LINES );

	for ( int i = 0; i < segments-1; ++i )
	{
		float currAngle = startAngle + deltaAngle * i;
		float nextAngle = startAngle + deltaAngle * (i+1);

		float x0 = center.x + radius * cosf( currAngle );
		float y0 = center.y + radius * sinf( currAngle );
		float x1 = center.x + radius * cosf( nextAngle );
		float y1 = center.y + radius * sinf( nextAngle );

		glVertex2f( x0, y0 );
		glVertex2f( x1, y1 );
	}

	glEnd();
}

void DrawTriangle( const v2 v0, const v2 v1, const v2 v2 )
{
	CHECK_SHADER;

	glBegin( GL_TRIANGLES );
	glVertex2f( v0.x, v0.y );
	glVertex2f( v1.x, v1.y );
	glVertex2f( v2.x, v2.y );
	glEnd();
}

void DrawTriangleWire( const v2 v0, const v2 v1, const v2 v2 )
{
	CHECK_SHADER;

	glBegin( GL_LINE_LOOP );
	glVertex2f( v0.x, v0.y );
	glVertex2f( v1.x, v1.y );
	glVertex2f( v2.x, v2.y );
	glEnd();
}

void DrawTriangleWire( const v3 v0, const v3 v1, const v3 v2 )
{
	CHECK_SHADER;

	glBegin( GL_LINE_LOOP );
	glVertex3f( v0.x, v0.y, v0.z );
	glVertex3f( v1.x, v1.y, v1.z );
	glVertex3f( v2.x, v2.y, v2.z );
	glEnd();
}

void DrawTriangle( const v3 v0, const v3 v1, const v3 v2 )
{
	CHECK_SHADER;

	glBegin( GL_TRIANGLES );
	glVertex3f( v0.x, v0.y, v0.z );
	glVertex3f( v1.x, v1.y, v1.z );
	glVertex3f( v2.x, v2.y, v2.z );
	glEnd();
}


void DrawRectTexCoords( const v2 bottomLeft, const v2 dimen, v2 uv0, v2 uv1 )
{
	CHECK_SHADER;

	glBegin( GL_QUADS );

	glTexCoord2f( uv1.x, uv1.y );
	glVertex2f( bottomLeft.x+dimen.x, bottomLeft.y+dimen.y );

	glTexCoord2f( uv0.x, uv1.y );
	glVertex2f( bottomLeft.x, bottomLeft.y+dimen.y );

	glTexCoord2f( uv0.x, uv0.y );
	glVertex2f( bottomLeft.x, bottomLeft.y );

	glTexCoord2f( uv1.x, uv0.y );
	glVertex2f( bottomLeft.x+dimen.x, bottomLeft.y );

	glEnd();
}

void DrawTex2D( const v2 topLeft, const v2 dimen, StrongHandle<Texture> tex )
{
	CHECK_SHADER;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

	tex->Bind();

	glBegin( GL_QUADS );

	glTexCoord2f( 1.0f, 1.0f );
	glVertex2f( topLeft.x+dimen.x, topLeft.y-dimen.y );

	glTexCoord2f( 0.0f, 1.0f );
	glVertex2f( topLeft.x, topLeft.y-dimen.y );
	
	glTexCoord2f( 0.0f, 0.0f );
	glVertex2f( topLeft.x, topLeft.y );

	glTexCoord2f( 1.0f, 0.0f );
	glVertex2f( topLeft.x+dimen.x, topLeft.y );

	glEnd();

	tex->Unbind();
}

void DrawRectWire( const v2 bottomLeft, const v2 dimen )
{
	CHECK_SHADER;

	glBegin( GL_LINE_LOOP );
	glVertex2f( bottomLeft.x + dimen.x, bottomLeft.y  );
	glVertex2f( bottomLeft.x, bottomLeft.y );
	glVertex2f( bottomLeft.x, bottomLeft.y + dimen.y );
	glVertex2f( bottomLeft.x + dimen.x, bottomLeft.y  + dimen.y);
	glEnd();
}

void RoundedRectHelper( float startAngleRad, float radius, v2 startPos )
{
	const int numRoundedVerts = 4;
	const float roundedCornerAngle = PI*0.5f;
	const float roundedCircleDeltaAngle = roundedCornerAngle / (numRoundedVerts-1);

	for(int i = 0; i < numRoundedVerts; ++i )
	{
		float angleRad = startAngleRad + i*roundedCircleDeltaAngle;
		v2 pos = startPos + unitCircle(angleRad)*radius;
		glVertex2f(pos.x, pos.y);
	}
}

void DrawRectRounded( const v2 bottomLeft, const v2 dimen, float cornerRadius )
{
	CHECK_SHADER;	

	glBegin( GL_TRIANGLE_FAN );

	// center
	const v2 centerPos = bottomLeft + dimen*0.5f;
	glVertex2f(centerPos.x, centerPos.y);
	
	RoundedRectHelper( PI, cornerRadius, bottomLeft + v2(cornerRadius) ); // bottom left
	RoundedRectHelper( 1.5f*PI, cornerRadius, bottomLeft + v2(dimen.x-cornerRadius, cornerRadius) ); // bottom right
	RoundedRectHelper( 0.0f, cornerRadius, bottomLeft + dimen - v2(cornerRadius) ); // top right
	RoundedRectHelper( PI*0.5f, cornerRadius, bottomLeft + v2(cornerRadius, dimen.y-cornerRadius) ); // top right

	// back to start
	v2 finalPos = bottomLeft + v2(0.0f, cornerRadius);
	glVertex2f( finalPos.x, finalPos.y );

	glEnd();
}

void DrawRectRoundedTop( const v2 bottomLeft, const v2 dimen, float cornerRadius )
{
	CHECK_SHADER;	

	glBegin( GL_TRIANGLE_FAN );

	// center
	const v2 centerPos = bottomLeft + dimen*0.5f;
	glVertex2f(centerPos.x, centerPos.y);
	
	glVertex2f(bottomLeft.x, bottomLeft.y); // bottom left
	glVertex2f(bottomLeft.x + dimen.x, bottomLeft.y); // bottom right
	RoundedRectHelper( 0.0f, cornerRadius, bottomLeft + dimen - v2(cornerRadius) ); // top right
	RoundedRectHelper( PI*0.5f, cornerRadius, bottomLeft + v2(cornerRadius, dimen.y-cornerRadius) ); // top right
	glVertex2f(bottomLeft.x, bottomLeft.y); // bottom left, back to start

	glEnd();
}

void DrawRectRoundedTopWire( const v2 bottomLeft, const v2 dimen, float cornerRadius )
{
	CHECK_SHADER;	

	glBegin( GL_LINE_LOOP );
	
	glVertex2f(bottomLeft.x, bottomLeft.y); // bottom left
	glVertex2f(bottomLeft.x + dimen.x, bottomLeft.y); // bottom right
	RoundedRectHelper( 0.0f, cornerRadius, bottomLeft + dimen - v2(cornerRadius) ); // top right
	RoundedRectHelper( PI*0.5f, cornerRadius, bottomLeft + v2(cornerRadius, dimen.y-cornerRadius) ); // top right

	glEnd();
}


void DrawRectRoundedBot( const v2 bottomLeft, const v2 dimen, float cornerRadius )
{
	CHECK_SHADER;	

	glBegin( GL_TRIANGLE_FAN );

	// center
	const v2 centerPos = bottomLeft + dimen*0.5f;
	glVertex2f(centerPos.x, centerPos.y);
	
	RoundedRectHelper( PI, cornerRadius, bottomLeft + v2(cornerRadius) ); // bottom left
	RoundedRectHelper( 1.5f*PI, cornerRadius, bottomLeft + v2(dimen.x-cornerRadius, cornerRadius) ); // bottom right
	glVertex2f(bottomLeft.x+dimen.x, bottomLeft.y+dimen.y); // top right
	glVertex2f(bottomLeft.x, bottomLeft.y+dimen.y); // top left

	// back to start
	v2 finalPos = bottomLeft + v2(0.0f, cornerRadius);
	glVertex2f( finalPos.x, finalPos.y );

	glEnd();
}

void DrawRectRoundedBotWire( const v2 bottomLeft, const v2 dimen, float cornerRadius )
{
	CHECK_SHADER;	

	glBegin( GL_LINE_LOOP );

	// center
	const v2 centerPos = bottomLeft + dimen*0.5f;
	
	RoundedRectHelper( PI, cornerRadius, bottomLeft + v2(cornerRadius) ); // bottom left
	RoundedRectHelper( 1.5f*PI, cornerRadius, bottomLeft + v2(dimen.x-cornerRadius, cornerRadius) ); // bottom right
	glVertex2f(bottomLeft.x+dimen.x, bottomLeft.y+dimen.y); // top right
	glVertex2f(bottomLeft.x, bottomLeft.y+dimen.y); // top left

	glEnd();
}

void DrawRectRoundedWire( const v2 bottomLeft, const v2 dimen, float cornerRadius )
{
	CHECK_SHADER;

	glBegin( GL_LINE_LOOP );

	RoundedRectHelper( PI, cornerRadius, bottomLeft + v2(cornerRadius) ); // bottom left
	RoundedRectHelper( 1.5f*PI, cornerRadius, bottomLeft + v2(dimen.x-cornerRadius, cornerRadius) ); // bottom right
	RoundedRectHelper( 0.0f, cornerRadius, bottomLeft + dimen - v2(cornerRadius) ); // top right
	RoundedRectHelper( PI*0.5f, cornerRadius, bottomLeft + v2(cornerRadius, dimen.y-cornerRadius) ); // top right

	glEnd();
}

void DrawCube( const v3 dimen )
{
	CHECK_SHADER;

	glBegin( GL_QUADS );

	// z-pos
	glVertex3f( -dimen.x, -dimen.y, +dimen.z );
	glVertex3f( -dimen.x, +dimen.y, +dimen.z );
	glVertex3f( +dimen.x, +dimen.y, +dimen.z );
	glVertex3f( +dimen.x, -dimen.y, +dimen.z );

	// z-neg
	glVertex3f( -dimen.x, -dimen.y, -dimen.z );
	glVertex3f( -dimen.x, +dimen.y, -dimen.z );
	glVertex3f( +dimen.x, +dimen.y, -dimen.z );
	glVertex3f( +dimen.x, -dimen.y, -dimen.z );

	// x-neg
	glVertex3f( -dimen.x, -dimen.y, -dimen.z );
	glVertex3f( -dimen.x, -dimen.y, +dimen.z );
	glVertex3f( -dimen.x, +dimen.y, +dimen.z );
	glVertex3f( -dimen.x, +dimen.y, -dimen.z );

	// x-pos
	glVertex3f( +dimen.x, -dimen.y, -dimen.z );
	glVertex3f( +dimen.x, -dimen.y, +dimen.z );
	glVertex3f( +dimen.x, +dimen.y, +dimen.z );
	glVertex3f( +dimen.x, +dimen.y, -dimen.z );

	// y-neg
	glVertex3f( -dimen.x, -dimen.y, -dimen.z );
	glVertex3f( -dimen.x, -dimen.y, +dimen.z );
	glVertex3f( +dimen.x, -dimen.y, +dimen.z );
	glVertex3f( +dimen.x, -dimen.y, -dimen.z );

	// y-pos
	glVertex3f( -dimen.x, +dimen.y, -dimen.z );
	glVertex3f( -dimen.x, +dimen.y, +dimen.z );
	glVertex3f( +dimen.x, +dimen.y, +dimen.z );
	glVertex3f( +dimen.x, +dimen.y, -dimen.z );

	glEnd();
}

void DrawCubeWire( const v3 dimen )
{
	CHECK_SHADER;

	glBegin( GL_LINES );

	// z-pos
	glVertex3f( -dimen.x, -dimen.y, +dimen.z );
	glVertex3f( -dimen.x, +dimen.y, +dimen.z );
	glVertex3f( -dimen.x, +dimen.y, +dimen.z );
	glVertex3f( +dimen.x, +dimen.y, +dimen.z );
	glVertex3f( +dimen.x, +dimen.y, +dimen.z );
	glVertex3f( +dimen.x, -dimen.y, +dimen.z );
	glVertex3f( +dimen.x, -dimen.y, +dimen.z );
	glVertex3f( -dimen.x, -dimen.y, +dimen.z );

	// z-neg
	glVertex3f( -dimen.x, -dimen.y, -dimen.z );
	glVertex3f( -dimen.x, +dimen.y, -dimen.z );
	glVertex3f( -dimen.x, +dimen.y, -dimen.z );
	glVertex3f( +dimen.x, +dimen.y, -dimen.z );
	glVertex3f( +dimen.x, +dimen.y, -dimen.z );
	glVertex3f( +dimen.x, -dimen.y, -dimen.z );
	glVertex3f( +dimen.x, -dimen.y, -dimen.z );
	glVertex3f( -dimen.x, -dimen.y, -dimen.z );

	// sides
	glVertex3f( -dimen.x, -dimen.y, -dimen.z );
	glVertex3f( -dimen.x, -dimen.y, +dimen.z );
	glVertex3f( -dimen.x, +dimen.y, -dimen.z );
	glVertex3f( -dimen.x, +dimen.y, +dimen.z );
	glVertex3f( +dimen.x, +dimen.y, -dimen.z );
	glVertex3f( +dimen.x, +dimen.y, +dimen.z );
	glVertex3f( +dimen.x, -dimen.y, -dimen.z );
	glVertex3f( +dimen.x, -dimen.y, +dimen.z );

	glEnd();
}

void DrawScreenUV( v2 topLeftUV, v2 dimenUV )
{
	CHECK_SHADER;

	topLeftUV = topLeftUV *2.0f - v2(1.0f);
	topLeftUV.y *= -1.0f;
	dimenUV *= 2.0f;

	glDisable(GL_DEPTH_TEST);

	v2 tl = v2( topLeftUV.x+dimenUV.x, topLeftUV.y-dimenUV.y );
	v2 tr = v2( topLeftUV.x, topLeftUV.y-dimenUV.y );
	v2 br = v2( topLeftUV.x, topLeftUV.y );
	v2 bl = v2( topLeftUV.x+dimenUV.x, topLeftUV.y );

	glBegin( GL_QUADS );

	glTexCoord2f( 1.0f, 0.0f );
	glVertex2f( tl.x, tl.y );

	glTexCoord2f( 0.0f, 0.0f );
	glVertex2f( tr.x, tr.y );

	glTexCoord2f( 0.0f, 1.0f );
	glVertex2f( br.x, br.y  );

	glTexCoord2f( 1.0f, 1.0f );
	glVertex2f( bl.x, bl.y );

	glEnd();
}

void DrawMayaPlane( int gridsWidth )
{
	glBegin( GL_LINES );

	float start = -gridsWidth * 0.5f;

	for( int x = 0; x < gridsWidth; ++x )
	{
		for( int z = 0; z < gridsWidth; ++z )
		{
			glVertex3f( start+x, 0, start+z );
			glVertex3f( start+x+1, 0, start+z );
			glVertex3f( start+x, 0, start+z );
			glVertex3f( start+x, 0, start+z+1 );
		}
	}

	glVertex3f( -start, 0, start );
	glVertex3f( -start, 0, -start );

	glVertex3f( start, 0, -start );
	glVertex3f( -start, 0, -start );

	glEnd();
}

void DrawSpline( const v2 v0, const v2 tan0, const v2 v1, const v2 tan1, const float t0 /*= 0.0f*/, const float t1 /*= 1.0f */, int numSteps )
{
	const float deltaT = (t1-t0)/numSteps;

	glBegin( GL_LINE_STRIP );

	for( int i = 0; i < numSteps; ++i )
	{
		const float t = t0 + deltaT*i;
		v2 pos = hermite(v0, tan0, v1, tan1, t );

		glVertex2f( pos.x, pos.y );
	}
	glEnd();
}

void DrawSpline( const v3 v0, const v3 tan0, const v3 v1, const v3 tan1, const float t0 /*= 0.0f*/, const float t1 /*= 1.0f*/, int numSteps /*= 64 */ )
{
	const float deltaT = (t1-t0)/numSteps;

	glBegin( GL_LINE_STRIP );

	for( int i = 0; i < numSteps; ++i )
	{
		const float t = t0 + deltaT*i;
		v3 pos = hermite(v0, tan0, v1, tan1, t );

		glVertex3f( pos.x, pos.y, pos.z );
	}
	glEnd();
}

}