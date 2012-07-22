#define V3_CHECK(v) assert(v)

inline v3::v3() : x(0), y(0), z(0) {;}
inline v3::v3(float v) : x(v), y(v), z(v) {;}
inline v3::v3(float _x, float _y, float _z ) : x(_x), y(_y), z(_z) {;}

inline v3& v3::operator+=( const v3& o )
{
	x += o.x;
	y += o.y;
	z += o.z;
	V3_CHECK( check(*this) );
	return *this;
}

inline v3& v3::operator-=( const v3& o )
{
	x -= o.x;
	y -= o.y;
	z -= o.z;
	V3_CHECK( check(*this) );
	return *this;
}

inline v3& v3::operator*=( const v3& o )
{
	x *= o.x;
	y *= o.y;
	z *= o.z;
	V3_CHECK( check(*this) );
	return *this;
}

inline v3& v3::operator/=( const v3& o )
{
	V3_CHECK( fabsf(o.x) > FLT_MIN );
	V3_CHECK( fabsf(o.y) > FLT_MIN );
	V3_CHECK( fabsf(o.z) > FLT_MIN );

	x /= o.x;
	y /= o.y;
	z /= o.z;
	
	V3_CHECK( check(*this) );
	return *this;
}

inline v3& v3::operator*=( float s )
{
	x *= s;
	y *= s;
	z *= s;
	V3_CHECK( check(*this) );
	return *this;
}

inline v3& v3::operator/=( float s )
{
	float div = 1.0f/s;
	x *= div;
	y *= div;
	z *= div;
	V3_CHECK( check(*this) );
	return *this;
}

inline v3 v3::operator*( const v3& o ) const
{
	V3_CHECK( check(o) );
	V3_CHECK( check(*this) );
	return v3(x*o.x, y*o.y, z*o.z);
}

inline v3 v3::operator/( const v3& o ) const
{
	V3_CHECK( check(o) );
	V3_CHECK( check(*this) );
	V3_CHECK( fabsf(o.x) > FLT_MIN );
	V3_CHECK( fabsf(o.y) > FLT_MIN );
	V3_CHECK( fabsf(o.z) > FLT_MIN );
	return v3(x/o.x, y/o.y, z/o.z);
}

inline v3 v3::operator+( const v3& o ) const
{
	V3_CHECK( check(o) );
	V3_CHECK( check(*this) );
	return v3(x+o.x, y+o.y, z+o.z);
}

inline v3 v3::operator-( const v3& o ) const
{
	V3_CHECK( check(o) );
	V3_CHECK( check(*this) );
	return v3(x-o.x, y-o.y, z-o.z);
}

inline v3 v3::operator-() const
{
	V3_CHECK( check(*this) );
	return v3(-x, -y, -z);
}

inline v3 v3::operator*( float s ) const
{
	V3_CHECK( check(*this) );
	return v3(x*s, y*s, z*s);
}

inline v3 v3::operator/( float s ) const
{
	V3_CHECK( fabsf(s) > FLT_MIN );
	V3_CHECK( check(*this) );
	float div = 1.0f / s;
	return v3(x*div, y*div, z*div);
}

inline bool	v3::operator==( const v3& o ) const
{
	return x == o.x && y == o.y && z == o.z;
}
inline bool v3::operator!=( const v3& o ) const
{
	return !(*this == o);
}

inline bool check( const v3& v )
{
	return v.x<=FLT_MAX && v.x>=-FLT_MAX && v.y<=FLT_MAX && v.y>=-FLT_MAX && v.z<=FLT_MAX && v.z>=-FLT_MAX;
}

inline float length( const v3& v )
{
	return sqrtf( lengthSqr(v) );
}

inline float lengthSqr( const v3& v )
{
	return v.x*v.x + v.y*v.y + v.z*v.z;
}

inline float dot( const v3& a, const v3& b )
{
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

inline float distance( const v3& p0, const v3& p1 )
{
	return length( p1 - p0 );
}

inline v3 operator*( float s, const v3& v )
{
	return v3( v.x*s, v.y*s, v.z*s );
}

inline v3 cross( const v3& a, const v3& b)
{
	v3 res( a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x );
	V3_CHECK( check(res) );
	return res;
}

inline v3 normalize( const v3& v )
{
	float len = length(v);
	V3_CHECK( fabs(len) > FLT_MIN );
	float divLen = 1.0f / len;
	return v * divLen;
}

inline v3 max( const v3& a, const v3& b )
{
	return v3( max(a.x, b.x), max(a.y, b.y), max(a.z, b.z) );
}

inline v3 min( const v3& a, const v3& b )
{
	return v3( min(a.x, b.x), min(a.y, b.y), min(a.z, b.z) );
}

inline v3 abs( const v3& v )
{
	return v3( fabs(v.x), fabs(v.y), fabs(v.z) );
}

inline v3 lerp( const v3& x, const v3& y, float a )
{
	return v3( lerp(x.x, y.x, a), lerp( x.y, y.y, a), lerp( x.z, y.z, a) );
}

inline v3 clamp( const v3& v, const v3& min, const v3& max )
{
	return v3( clamp(v.x, min.x, max.x), clamp(v.y, min.y, max.y), clamp(v.z, min.z, max.z) );
}

inline v3 saturate( const v3& v )
{
	return v3( clamp(v.x, 0.0f, 1.0f), clamp(v.y, 0.0f, 1.0f ), clamp(v.z, 0.0f, 1.0f ) );
}

inline v3 hermite( const v3& v0, const v3& tan0, const v3& v1, const v3& tan1, float t )
{
	float t_cubed = t*t*t;
	float t_sqr = t*t;

	float h1 =  2.0f*t_cubed - 3.0f*t_sqr + 1.0f;   // calculate basis function 1
	float h2 = -2.0f*t_cubed + 3.0f*t_sqr;          // calculate basis function 2
	float h3 =  t_cubed - 2.0f*t_sqr + t;			// calculate basis function 3
	float h4 =  t_cubed -  t_sqr;					// calculate basis function 4

	return h1*v0 + h2*v1 + h3*tan0 + h4*tan1;
}