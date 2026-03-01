/**********************************************************************
 *<
	FILE: ffdui.h

	DESCRIPTION:

	CREATED BY: Ravi Karra

	HISTORY: created 1/11/99

 *>	Copyright (c) 1996 Rolf Berteig, All Rights Reserved.
 **********************************************************************/

#ifndef __FFDUI__H
#define __FFDUI__H

#include "ffdmod.h"
#include "ActionTable.h"

// The reference versions
#define ES_REF_VER_0	0	// Pre-r3 (vertex controllers are only refs)
#define ES_REF_VER_1	1	// MAXr3 (ref 0 is Point Controller Container)

// Reference indices
#define TM_REF		0
#define PBLOCK_REF	1
#define POINTCTRLCONTAINER_REF	2

// Selection levels
#define SEL_OBJECT		0
#define SEL_POINTS		1
#define SEL_LATTICE		2
#define SEL_SETVOLUME	3

// Right Click ID's
enum { RC_ANIMATE_ALL = SEL_SETVOLUME+1, RC_ALLX, RC_ALLY, RC_ALLZ };

// Keyboard Shortcuts stuff
const ActionTableId kFFDActions = 0x7ed73ca2;
const ActionContextId kFFDContext = 0x7ed73ca2;

#define NumElements(array) (sizeof(array) / sizeof(array[0]))

ActionTable* GetActions();

template <class T>
class FFDActionCB : public ActionCallback
{
	public:
		T*		ffd;
				FFDActionCB(T *ffd) { this->ffd = ffd; }
		BOOL	ExecuteAction(int id); 
};

class sMyEnumProc : public DependentEnumProc 
	{
      public :
		INodeTab Nodes;              
		virtual int proc(ReferenceMaker *rmaker); 
	};


template <class T>
BOOL FFDActionCB<T>::ExecuteAction(int id) {
	switch (id) {
		case ID_SUBOBJ_TOP:
			ffd->ip->SetSubObjectLevel(SEL_OBJECT);
			ffd->ip->RedrawViews(ffd->ip->GetTime());
			return TRUE;
		case ID_SUBOBJ_CP:
			ffd->ip->SetSubObjectLevel(SEL_POINTS);
			return TRUE;
		case ID_SUBOBJ_LATTICE:
			ffd->ip->SetSubObjectLevel(SEL_LATTICE);
			return TRUE;
		case ID_SUBOBJ_SETVOLUME:
			ffd->ip->SetSubObjectLevel(SEL_SETVOLUME);
			return TRUE;
	}
	return FALSE;
	
}

#endif //__FFDUI__H
