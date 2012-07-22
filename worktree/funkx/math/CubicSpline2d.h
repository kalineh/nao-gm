#ifndef _INCLUDE_CUBIC_SPLINE_2D_H
#define _INCLUDE_CUBIC_SPLINE_2D_H

#include <gm/gmBindHeader.h>
#include <common/HandledObj.h>
#include <common/StrongHandle.h>

#include <vector>
#include "v2.h"

namespace funk
{
	class CubicSpline2d : public funk::HandledObj<CubicSpline2d>
	{
	public:
		v2 CalcPosAt(float t) const;

		// control points
		int NumControlPts() const;
		const v2 GetControlPt( int i ) const;
		void AddControlPt( v2 pt );
		void SetControlPt( int i, v2 val );
		void RemoveControlPt(); // the last one
		void SortControlPtsByX();
		void SortControlPtsByY();

		GM_BIND_TYPEID(CubicSpline2d);

	private:

		float GetAlphaAt(int i) const;
		v2 GetTangentAt(int i) const;

		std::vector<v2> m_controlPts;
	};

	GM_BIND_DECL(CubicSpline2d);
}

#endif