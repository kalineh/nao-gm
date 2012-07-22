#ifndef _INCLUDE_GM_UTIL_EX_H_
#define _INCLUDE_GM_UTIL_EX_H_

#include <vector>

class gmMachine;
class gmTableObject;
struct gmVariable;

int gmCompileStr( gmMachine *vm, const char* file );
int gmSaveTableToFile( gmTableObject * table, const char * file );

// sorts table's children and outputs
void gmSortTableKeys( gmTableObject * table, std::vector<gmVariable*> & result );

#endif