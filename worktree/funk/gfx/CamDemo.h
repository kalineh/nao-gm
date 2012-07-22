#ifndef _INCLUDE_CAM_DEMO_H_
#define _INCLUDE_CAM_DEMO_H_

#include "Cam3d.h"
#include <common/StrongHandle.h>

namespace funk
{
	class CamDemo
	{
	public:
		void Init();
		void Begin();
		void End();
		void Gui();

		void Update( float dt );

		v3 GetPos() const { return m_cam.GetPos(); }
		void SetFov( float fov ) { m_fov = fov; }

	private:

		float m_totalTime;

		// camera
		float m_camHeight;
		float m_camRadius;
		float m_camAngleRad;
		float m_fov;
		float m_aspect;
		float m_focusY;

		bool m_autoSpinCamera;

		Cam3d m_cam;

	};
}

#endif