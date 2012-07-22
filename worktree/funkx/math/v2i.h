#ifndef _INCLUDE_v2i_H_
#define _INCLUDE_v2i_H_

#include <assert.h>
#include <math.h>
#include <float.h>
#include <math/Util.h>

namespace funk
{
class v2i
{
public:
	union
	{
		struct { int x, y; };
	};

	v2i();
	v2i( int v );
	v2i( int _re, int _im );
	v2i( const v2i &c )  : x(c.x), y(c.y) {;}

	int&			operator[]( size_t i )			{assert( i < 2 ); return (&x)[i];}
	const int&	operator[]( size_t i ) const		{assert( i < 2 ); return (&x)[i];}

	v2i&		operator+=( const v2i& o );
	v2i&		operator-=( const v2i& o );
	v2i&		operator*=( const v2i& o );
	v2i&		operator*=( int s );
	v2i&		operator/=( int s );
	
	v2i		operator*( const v2i& o ) const;
	v2i		operator/( const v2i& o ) const;
	v2i		operator+( const v2i& o ) const;
	v2i		operator-( const v2i& o ) const;
	v2i		operator-() const;
	v2i		operator*( int s ) const;
	v2i		operator/( int s ) const;

	bool	operator==( const v2i& o ) const;
	bool	operator!=( const v2i& o ) const;
};

float	length( const v2i& v );
int		lengthSqr( const v2i& v );;

v2i	operator*( int s, const v2i& v );

#include "v2i.inl"
}

#endif