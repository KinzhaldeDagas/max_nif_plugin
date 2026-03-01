/**********************************************************************
 *<
	FILE: BasicOps.h

	DESCRIPTION:

	CREATED BY: Steve Anderson, based on mods.h

	HISTORY: Created 2001

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#ifndef __BASIC_OPS__H
#define __BASIC_OPS__H

#include "Max.h"
#include "resource.h"
#include "meshadj.h"

// Modifiers hereabouts:
extern ClassDesc* GetVertexWeldModDesc();
extern ClassDesc* GetSymmetryModDesc();
extern ClassDesc* GetSliceModDesc();

TCHAR *GetString(int id);

// in BasicOps.cpp
extern HINSTANCE hInstance;


#define BIGFLOAT	float(9999999)

class BasicModData : public LocalModData
{
private:
	float			size;		// Gizmo Size
	AdjEdgeList*	ael;		// Cached Edge List (for meshes)

public:
	BasicModData() {
		size = -1;
		ael = nullptr;
	}

	~BasicModData() {
		if (ael)
			delete ael;
	}

	LocalModData *Clone() {
		return new BasicModData;
	}

	// Size Accessors
	float	GetSize()
	{
		return size;
	}

	void	SetSize(float s)
	{
		size = s;
	}

	// EdgeList Accessors
	void	CacheEdges(Mesh &m)
	{
		if (ael)
			delete ael;

		ael = new AdjEdgeList(m);
	}

	void	ClearEdgeList()
	{
		if (ael)
			delete ael;

		ael = nullptr;
	}

	AdjEdgeList* GetEdgeList()
	{
		return ael;
	}

	bool CheckEdgeList(int Count)
	{
		if ((!ael) || (ael->nverts != Count))
			return false;

		return true;
	}
};

#endif

