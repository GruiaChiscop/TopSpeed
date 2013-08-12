#include <Windows.h>
#include <DxCommon/If/Common.h>
#include <Common/If/Common.h>


#ifndef DXCOMMON_STATIC

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}

#endif

_dxcommon_ Tracer  dxTracer("DX");
