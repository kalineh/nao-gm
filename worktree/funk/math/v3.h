#ifndef _INCLUDE_V3_H_
#define _INCLUDE_V3_H_

#include <assert.h>
#include <math.h>
#include <float.h>
#include <math/Util.h>
#include <gm/gmVector.h>

namespace funk
{
class v3
{
public:

	union
	{
		struct { float x, y, z; };
		gmVec3 gmv3;
	};

	v3();
	v3( const gmVec3 &_gmv3 ) { gmv3 = _gmv3; }
	v3( float v );
	v3( float _x, float _y, float _z );

	float&			operator[]( size_t i )				{assert( i < 3 ); return (&x)[i];}
	const float&	operator[]( size_t i ) const		{assert( i < 3 ); return (&x)[i];}

	v3&		operator+=( const v3& o );
	v3&		operator-=( const v3& o );
	v3&		operator*=( const v3& o );
	v3&		operator/=( const v3& o );
	v3&		operator*=( float s );
	v3&		operator/=( float s );
	
	v3		operator*( const v3& o ) const;
	v3		operator/( const v3& o ) const;
	v3		operator+( const v3& o ) const;
	v3		operator-( const v3& o ) const;
	v3		operator-() const;
	v3		operator*( float s ) const;
	v3		operator/( float s ) const;

	bool	operator==( const v3& o ) const;
	bool	operator!=( const v3& o ) const;
};

//v3 HSVtoRGB( float hAngleDeg, float s, float v );

bool check( const v3& v );

float	length( const v3& v );
float	lengthSqr( const v3& v );
float	dot( const v3& a, const v3& b );
float	distance( const v3& p0, const v3& p1 );

v3	operator*( float s, const v3& v );

v3	cross( const v3& a, const v3& b);
v3	normalize( const v3& v );
v3	max( const v3& a, const v3& b );
v3	min( const v3& a, const v3& b );
v3	abs( const v3& v );
v3	lerp( const v3& x, const v3& y, float a );
v3	clamp( const v3& v, const v3& min, const v3& max );
v3	saturate( const v3& v );

v3 facenormal_cw( const v3& v0, const v3& v1, const v3& v3 );
v3 facenormal_ccw( const v3& v0, const v3& v2, const v3& v3 );

v3 hermite( const v3& v0, const v3& tan0, const v3& v1, const v3& tan1, float t );

#include "v3.inl"
}

#endif