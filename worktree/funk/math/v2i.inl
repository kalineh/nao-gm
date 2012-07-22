
inline v2i::v2i() : x(0), y(0) {;}
inline v2i::v2i(int v) : x(v), y(v) {;}
inline v2i::v2i(int _x, int _y ) : x(_x), y(_y) {;}

inline v2i& v2i::operator+=( const v2i& o )
{
	*this = (*this) + o;
	return *this;
}

inline v2i& v2i::operator-=( const v2i& o )
{
	*this = (*this) - o;
	return *this;
}

inline v2i& v2i::operator*=( const v2i& o )
{
	*this = (*this) * o;
	return *this;
}

inline v2i& v2i::operator*=( int s )
{
	x *= s;
	y *= s;
	return *this;
}

inline v2i& v2i::operator/=( int s )
{
	assert( s != 0 );
	x /= s;
	y /= s;
	return *this;
}

inline v2i v2i::operator*( const v2i& o ) const
{
	return v2i( x*o.x , y*o.y );
}

inline v2i v2i::operator/( const v2i& o ) const
{
	assert(o.x != 0 || o.y != 0);
	return v2i( x/o.x, y/o.y );
}

inline v2i v2i::operator+( const v2i& o ) const
{
	return v2i(x+o.x, y+o.y);
}

inline v2i v2i::operator-( const v2i& o ) const
{
	return v2i(x-o.x, y-o.y);
}

inline v2i v2i::operator-() const
{
	return v2i(-x, -y);
}

inline v2i v2i::operator*( int s ) const
{
	return v2i(x*s, y*s);
}

inline v2i v2i::operator/( int s ) const
{
	assert(s!=0);
	return v2i(x/s, y/s);
}

inline bool	v2i::operator==( const v2i& o ) const
{
	return x == o.x && y == o.y;
}

inline bool v2i::operator!=( const v2i& o ) const
{
	return !(*this == o);
}

inline float length( const v2i& v )
{
	return sqrtf( (float)lengthSqr(v) );
}

inline int lengthSqr( const v2i& v )
{
	return v.x*v.x + v.y*v.y;
}

inline v2i operator*( int s, const v2i& v )
{
	return v2i( v.x*s, v.y*s );
}