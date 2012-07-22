#include "v3.h"

namespace funk
{

v3 facenormal_cw( const v3& v0, const v3& v1, const v3& v2 )
{
	v3 dir0 = v1 - v0;
	v3 dir1 = v2 - v0;

	return normalize( cross( dir0, dir1 ) );
}

v3 facenormal_ccw( const v3& v0, const v3& v1, const v3& v2 )
{
	v3 dir0 = v1 - v0;
	v3 dir1 = v2 - v0;

	return normalize( cross( dir1, dir0 ) );
}

}
