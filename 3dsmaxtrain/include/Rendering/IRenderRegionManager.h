//////////////////////////////////////////////////////////////////////////////
//
//  Copyright 2015 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license
//  agreement provided at the time of installation or download, or which
//  otherwise accompanies this software in either electronic or hard copy form.
//
//////////////////////////////////////////////////////////////////////////////

/**********************************************************************
	DESCRIPTION:	Interface for the render region manager singleton.
 **********************************************************************/

#pragma once

#include "../maxtypes.h"
#include <Geom/box2.h>
#include "../ifnpub.h"
#include "../GetCOREInterface.h"
#include "../RendType.h"

 //-----------------------------------------------------------------------------
// core interface ID
#define RENDER_REGION_MANAGER_INTERFACE_ID Interface_ID(0x4881007a, 0x725d2e7b)

/*! \brief Represents access to render region editing for the viewport and VFB.*/
class IRenderRegionManager : public FPStaticInterface
{
public:
	// Functions
	/*! \brief Query whether the system is currently editing a region. */
	virtual bool IsEditing() = 0;
	/*! \brief Query whether the current render mode supports region render editing. */
	virtual bool IsRegionEditable() = 0;
	/*! \brief Pushes the region command mode on to the command stack to start region editing.
				This action will switch to a region editable mode if the current render mode
				does not support regions. */
	virtual void BeginRegionEdit() = 0;
	/*! \brief Pops the command mode off of the command stack to leave region editing. */
	virtual void EndRegionEdit() = 0;
	/*! \brief Helper that calls Begin/EndRegion to toggle between enabled and disabled. */
	virtual void ToggleRegionEdit() = 0;
	/*! \brief Handler called when region editing has started. */
	virtual void OnBeginRegionEdit() = 0;
	/*! \brief Handler called when region editing has ended */
	virtual void OnEndRegionEdit() = 0;
	/*! \brief Handler called when the region or related conditions have changed.
		This will notify region clients to update their display. This may also 
		end editing if it is no longer valid (e.g. region render mode is no longer selected). */
	virtual void OnRegionSynchronize() = 0;
	/*! \brief Get the render region (crop or blowup, depending on the current mode) in 0-1.0 space.*/
	virtual FBox2 GetRegionRectangle() = 0;
	/*! \brief Get the render region for a specific mode (crop or blowup) for a specific viewport.
	\param viewID The viewport id.
	\param renderType RENDER_CROP or RENDER_BLOWUP.
	*/
	virtual FBox2 GetRegionRectangle(int viewID, RenderUIType renderType) = 0;
	/*! \brief Set the render region (crop or blowup, depending on the current mode) in 0-1.0 space.
		This method creates a restore object if the Hold is holding.
	\param region The render region.
	*/
	virtual void SetRegionRectangle(FBox2 region) = 0;
	/*! \brief Set the render region for a specific mode (crop or blowup) for a specific viewport.
	\param viewID The viewport id.
	\param renderType RENDER_CROP or RENDER_BLOWUP.
	\param region The render region.
	*/
	virtual void SetRegionRectangle(int viewID, RenderUIType renderType, FBox2 region) = 0;
	/*! \brief Mark the current region values for undo if the Hold is holding. */
	virtual void PutRenderRegionUndo() = 0;
	/*! \brief Whether the region manager is the current VFB crop handler. */
	virtual bool IsVFBCropCallback() = 0;
};

//-----------------------------------------------------------------------------
/*! \brief Acquire a pointer to the render region manager.
	\return A pointer to the manager. NULL on error. */
inline IRenderRegionManager* GetIRenderRegionManager()
{
	IRenderRegionManager* manager = static_cast<IRenderRegionManager*>(GetCOREInterface(RENDER_REGION_MANAGER_INTERFACE_ID));
	DbgAssert(manager);
	return manager;
}

