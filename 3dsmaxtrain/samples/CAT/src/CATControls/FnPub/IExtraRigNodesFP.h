//**************************************************************************/
// Copyright (c) 2011 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include <ifnpub.h>

/**********************************************************************
 * IExtraRigNodesFP: Published functions for IExtraRigNodes.
 */

#pragma warning(push)
#pragma warning(disable:4238) // necessary for _BV types

#define I_EXTRARIGNODES_FP		Interface_ID(0x7b732429, 0x793a0de2)
class IExtraRigNodesFP : public FPMixinInterface {
public:

	// 2 methods so we can add and remove groups of nodes at a time
	virtual void AddExtraRigNodes(Tab<INode*> newnodes) = 0;
	virtual void RemoveExtraRigNodes(Tab<INode*> removednodes) = 0;

	// Returns the array of Extra Rig nodes that we are currently storing
	virtual Tab<INode*>	IGetExtraRigNodes() = 0;
	virtual void		ISetExtraRigNodes(Tab<INode*> nodes) = 0;

	enum {
		fnAddExtraRigNodes,
		fnRemoveExtraRigNodes,
		propGetExtraRigNodes,
		propSetExtraRigNodes,
	};

	BEGIN_FUNCTION_MAP
		VFN_1(fnAddExtraRigNodes, AddExtraRigNodes, TYPE_INODE_TAB_BV);
		VFN_1(fnRemoveExtraRigNodes, RemoveExtraRigNodes, TYPE_INODE_TAB_BV);
		PROP_FNS(propGetExtraRigNodes, IGetExtraRigNodes, propSetExtraRigNodes, ISetExtraRigNodes, TYPE_INODE_TAB_BV);
	END_FUNCTION_MAP

	FPInterfaceDesc* GetDesc();

	static FPInterfaceDesc* GetFnPubDesc();
};

#pragma warning(pop)