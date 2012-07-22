#ifndef _INCLUDE_OBJ_LOADER_H_
#define _INCLUDE_OBJ_LOADER_H_

#include <vector>
#include <math/v3.h>
#include <math/v2.h>

#include <common/StrongHandle.h>
#include "Vbo.h"

namespace funk
{
	class ObjLoader
	{
	public:

		ObjLoader();	
		
		StrongHandle<Vbo> CreateVbo(); // create vbo from file data

		void LoadFile( const char * file );
		void SetSwizzleZY( bool val ) { m_swizzleZY = val; }
		void SetGenerateNormals( bool val ) { m_bGenNormals = val; }

	private:

		// settings
		bool m_swizzleZY;
		bool m_bGenNormals;

		struct Face
		{
			static const int kMaxPts = 4;

			int pos[kMaxPts];
			int normal[kMaxPts];
			int uv[kMaxPts];

			Face()
			{
				for ( int i = 0; i < kMaxPts; ++i ) pos[i] = normal[i] = uv[i] = -1;
			}
		};

		struct VertPosNormalUV
		{
			v3 pos;
			v3 normal;
			v2 uv;
		};

		std::vector< v3 > m_vertPos;
		std::vector< v3 > m_vertNormal;
		std::vector< v2 > m_vertUV;
		std::vector< Face > m_faces;

		void GetVboVertPos( std::vector<VertPosNormalUV> & out ) const;

	};
}
#endif