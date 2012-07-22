#ifndef _INCLUDE_CAMBASE_H_
#define _INCLUDE_CAMBASE_H_

#include <math/v2.h>
#include <math/v3.h>

namespace funk
{
	class CamBase
	{
	public:

		CamBase();

		virtual void Begin();
		virtual void End();

		v3 GetPos() const { return m_eyePos; }
		void SetPos( const v3 pos ) { m_eyePos = pos; }
		void SetLookAt( const v3 lookAt ) { m_orient = normalize(lookAt-m_eyePos); }
		void SetNearFar( v2 zNearFar );

	protected:
		v2 m_zNearFar;

	private:
		v3 m_eyePos;
		v3 m_orient;
	};
}

#endif