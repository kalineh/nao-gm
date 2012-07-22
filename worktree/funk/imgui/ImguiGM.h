#ifndef _INCLUDE_IMGUI_GM_H_
#define _INCLUDE_IMGUI_GM_H_

class gmMachine;
class gmUserArray;
class gmTableObject;

namespace funk
{
	void gmBindImguiLib( gmMachine * a_machine );

	void ImguiOutputTable(gmTableObject* table, gmUserArray* selectArr, bool showFunctions );
}

#endif