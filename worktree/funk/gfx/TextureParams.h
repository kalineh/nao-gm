#ifndef _INCLUDE_TEXTURE_PARAMS_H_
#define _INCLUDE_TEXTURE_PARAMS_H_

#include <gl/glew.h>

namespace funk
{
	enum GLInternalFormat
	{
		FUNK_RGBA8 = GL_RGBA8,
		FUNK_RGBA16 = GL_RGBA16,
		FUNK_A8 = GL_ALPHA8,
		FUNK_A12 = GL_ALPHA12,
		FUNK_A16 = GL_ALPHA16,
		FUNK_A16F = GL_ALPHA16F_ARB,
		FUNK_A32F = GL_ALPHA32F_ARB
	};

	enum GLFormat
	{
		FUNK_RED = GL_RED,
		FUNK_GREEN = GL_GREEN,
		FUNK_BLUE = GL_BLUE,
		FUNK_ALPHA = GL_ALPHA,
		FUNK_RGB = GL_RGB,
		FUNK_RGBA = GL_RGBA,
		FUNK_LUMIN = GL_LUMINANCE,
		FUNK_LUMIN_ALPHA = GL_LUMINANCE_ALPHA
	};

	enum GLDataType
	{
		FUNK_UNSIGNED_BYTE = GL_UNSIGNED_BYTE, 
		FUNK_BYTE = GL_BYTE, 
		FUNK_BITMAP = GL_BITMAP, 
		FUNK_UNSIGNED_SHORT = GL_UNSIGNED_SHORT, 
		FUNK_SHORT = GL_SHORT, 
		FUNK_UNSIGNED_INT = GL_UNSIGNED_INT, 
		FUNK_INT = GL_INT, 
		FUNK_FLOAT = GL_FLOAT,
		FUNK_HALF_FLOAT = GL_HALF_FLOAT_ARB
	};

	struct TexParams
	{
		GLInternalFormat	internalFormat;
		GLFormat			format;
		GLDataType			dataType;

		TexParams() : 
			internalFormat(FUNK_RGBA8), 
			format(FUNK_RGBA), 
			dataType(FUNK_UNSIGNED_BYTE) {;}

		TexParams( GLInternalFormat _internalFormat, GLFormat _format, GLDataType _dataType  ) :
			internalFormat( _internalFormat ), format(_format), dataType(_dataType) {;}
				
	};
}
#endif