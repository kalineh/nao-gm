#ifndef _INCLUDE_LINE_GRAPH_H_
#define _INCLUDE_LINE_GRAPH_H_

#include <vector>

#include <math/v2.h>
#include <math/v2i.h>

#include <common/HandledObj.h>
#include <gm/gmBindHeader.h>

namespace funk
{
	class LineGraph : public HandledObj<LineGraph>
	{
	public:
		LineGraph( float minVal, float maxVal, v2i dimen, int numValsMaxs = 128 );

		void Draw( v2 pos );
		void DrawBG( v2 pos );
		void PushVal(float val);
		void PopVal();
		void Clear();

		void SetMinVal(float minVal) { m_minVal = minVal; }
		void SetMaxVal(float maxVal) { m_maxVal = maxVal; }

		v2i Dimen() const { return m_dimen; }

		GM_BIND_TYPEID(LineGraph);

	private:

		float m_minVal;
		float m_maxVal;
		v2i m_dimen;
		int m_numMaxVals;
		int m_indexFront;
		std::vector<float> m_vals;
	};

	GM_BIND_DECL(LineGraph);
}

#endif