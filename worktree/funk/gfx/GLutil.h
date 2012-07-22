#ifndef _INCLUDE_GL_UTIL_H_
#define _INCLUDE_GL_UTIL_H_

#include <gl/glew.h>

namespace funk
{
	// return 0 if failed
	GLuint LoadProgramFile( const char * file, GLuint type );
}
#endif