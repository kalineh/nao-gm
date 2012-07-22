#include "GLutil.h"

#include <common/Debug.h>
#include <common/Util.h>
#include <iostream>

GLuint funk::LoadProgramFile( const char * file, GLuint type )
{
	CHECK( type == GL_FRAGMENT_SHADER || type == GL_VERTEX_SHADER, "Unsupported shader program type 0x%X", type );
	GLuint id = glCreateShader(type);

	// keeps loading shader until it works
	while (true)
	{
		const char * src = TextFileRead(file);
		int length = strlen(src);
		glShaderSource(id, 1, &src, &length );
		delete src;

		glCompileShader(id);

		GLint err;
		glGetShaderiv( id, GL_COMPILE_STATUS, &err );

		// There was an error!
		if ( err == GL_FALSE )
		{
			char buffer[1024];
			GLsizei size =  sizeof(buffer);
			glGetShaderInfoLog( id, size, &size, buffer );

			MESSAGE_BOX("Shader Compile Error", "Shader '%s'\nCompile Error: %s", file, buffer );
		}
		
		else
		{
			return id;
		}
	}

	return 0;
}