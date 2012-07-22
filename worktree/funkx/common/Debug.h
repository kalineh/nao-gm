#ifndef _INCLUDE_DEBUG_H_
#define _INCLUDE_DEBUG_H_

namespace funk
{
#ifndef _DEBUG
#define CHECK(a,...) 0
#else
	void CHECK( bool cond, const char * format = 0, ... );
#endif // _DEBUG

#ifndef _DEBUG
#define WARN(a,...) 0
#else
	void WARN( bool cond, const char * format = 0, ... );
#endif // _DEBUG

void MESSAGE_BOX(const char * title, const char * format = 0, ... );
}
#endif