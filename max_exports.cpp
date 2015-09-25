#include <Windows.h>
#include <Max.h>
#include "SceneExp.h"
static HINSTANCE hInstance = GetModuleHandle(NULL);

#define LIBTHPS_CLASS 0xfa96366
#define LIBTHPS_MDL_CLASS 1

class CHCExpClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new CHCScnExp; }
	const TCHAR *	ClassName() { return _T("CHC Engine Export"); }
	SClass_ID		SuperClassID() { return SCENE_EXPORT_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(LIBTHPS_CLASS,LIBTHPS_MDL_CLASS); }
	const TCHAR* 	Category() { return _T("Scene Export");  }
};

static CHCExpClassDesc CHCExpDesc;


__declspec( dllexport ) const TCHAR *
LibDescription() { return _T("CHC Engine Exporter"); }

__declspec( dllexport ) int
LibNumberClasses() { return 1; }

__declspec( dllexport ) ClassDesc *
LibClassDesc(int i) {
	switch(i) {
		case 0: return &CHCExpDesc; break;
		default: return 0; break;
	}

}

// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG 
LibVersion() { return VERSION_3DSMAX; }

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}