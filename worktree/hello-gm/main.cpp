//
// main.cpp
//

#include "main.h"
#include "filters.h"
#include "opencv_tests.h"

void RegisterProjectLibs(gmMachine* vm)
{
    RegisterGmFiltersLib(vm);

	GM_BIND_INIT( GMALProxy, vm );
	GM_BIND_INIT( GMVideoDisplay, vm );
}

int main(int argc, char** argv)
{
	funk::Core app;

	app.HandleArgs(argc, argv);
	app.Init();
	app.Run();
	app.Deinit();

    return 0;
}
