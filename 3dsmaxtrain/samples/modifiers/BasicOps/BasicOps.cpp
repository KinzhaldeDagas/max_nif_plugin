/**********************************************************************
 *<
   FILE: BasicOps.cpp

   DESCRIPTION:   DLL implementation of modifiers

   CREATED BY: Rolf Berteig (based on prim.cpp)

   HISTORY: created 30 January 1995
			08/29/2020 - M. Kaustinen: Removal of unused classes

 *>   Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "BasicOps.h"

#include "3dsmaxport.h"

HINSTANCE hInstance;

#define MAX_MOD_OBJECTS 20
ClassDesc *classDescArray[MAX_MOD_OBJECTS];
int classDescCount = 0;

void initClassDescArray(void)
{
   if( !classDescCount )
   {
      classDescArray[classDescCount++] = GetVertexWeldModDesc();
      classDescArray[classDescCount++] = GetSymmetryModDesc();
	  classDescArray[classDescCount++] = GetSliceModDesc();
   }
}

/** public functions **/
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
   switch(fdwReason) {
      case DLL_PROCESS_ATTACH:
         MaxSDK::Util::UseLanguagePackLocale();
         hInstance = hinstDLL;
         DisableThreadLibraryCalls(hInstance);
         break;
    }
	return(TRUE);
}


//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR * LibDescription()
{
   return GetString(IDS_LIBDESCRIPTION);
}

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses()
{
   initClassDescArray();
   return classDescCount;
}

// russom - 05/07/01 - changed to use classDescArray
__declspec( dllexport ) ClassDesc* LibClassDesc(int i)
{
   initClassDescArray();

   if( i < classDescCount )
      return classDescArray[i];
   else
      return NULL;
}

// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG LibVersion() { return VERSION_3DSMAX; }

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer() {
   return 1;
}

TCHAR *GetString(int id) {
   static TCHAR buf[256];
   if (hInstance) return LoadString(hInstance, id, buf, _countof(buf)) ? buf : NULL;
   return NULL;
}
