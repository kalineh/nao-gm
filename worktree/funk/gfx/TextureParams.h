#ifndef _INCLUDE_TEXTURE_PARAMS_H_
#define _INCLUDE_TEXTURE_PARAMS_H_

#include <gl/glew.h>

namespace funk
{
	enum GLInternalFormat
	{
		RGBA8 = GL_RGBA8,
		RGBA16 = GL_RGBA16,
		A8 = GL_ALPHA8,
		A12 = GL_ALPHA12,
		A16 = GL_ALPHA16,
		A16F = GL_ALPHA16F_ARB,
		A32F = GL_ALPHA32F_ARB
	};

	enum GLFormat
	{
		RED = GL_RED,
		GREEN = GL_GREEN,
		BLUE = GL_BLUE,
		ALPHA = GL_ALPHA,
		RGB = GL_RGB,
		RGBA = GL_RGBA,
		LUMIN = GL_LUMINANCE,
		LUMIN_ALPHA = GL_LUMINANCE_ALPHA
	};

	enum GLDataType
	{
		UNSIGNED_BYTE = GL_UNSIGNED_BYTE, 
		BYTE = GL_BYTE, 
		BITMAP = GL_BITMAP, 
		UNSIGNED_SHORT = GL_UNSIGNED_SHORT, 
		SHORT = GL_SHORT, 
		UNSIGNED_INT = GL_UNSIGNED_INT, 
		INT = GL_INT, 
		FLOAT = GL_FLOAT,
		HALF_FLOAT = GL_HALF_FLOAT_ARB
	};

	struct TexParams
	{
		GLInternalFormat	internalFormat;
		GLFormat			format;
		GLDataType			dataType;

		TexParams() : 
			internalFormat(RGBA8), 
			format(RGBA), 
			dataType(UNSIGNED_BYTE) {;}

		TexParams( GLInternalFormat _internalFormat, GLFormat _format, GLDataType _dataType  ) :
			internalFormat( _internalFormat ), format(_format), dataType(_dataType) {;}
				
	};
}
#endif