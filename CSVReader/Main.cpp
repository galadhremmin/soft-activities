#include "App.h"

#ifdef WIN32_GUI
#include "Win32\SoftActivities\stdafx.h"
#include "Win32\SoftActivities\SoftActivities.h"

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	CSoftActivities gui(hInstance);
	return gui.run();
}
#else
int main(int argc, const char **argv) {
    CApp app(argc, argv);
    return app.run();
}
#endif