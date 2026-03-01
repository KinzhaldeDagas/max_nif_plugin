/**********************************************************************
 *<
	FILE: IRollupSettings

	DESCRIPTION: Rollup Window Settings Interface

	CREATED BY: Nikolai Sander

	HISTORY: created 8/8/00

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "maxheap.h"
#include "ifnpub.h"
#include "GetCOREInterface.h"

// forward declarations
class ICatRegistry;

#define EFFECTS_ROLLUPCFG_CLASSID Class_ID(0x559c705d, 0x32573fe3)
#define ATMOSPHERICS_ROLLUPCFG_CLASSID Class_ID(0x40ef5775, 0x5b5606f)
#define DISPLAY_PANEL_ROLLUPCFG_CLASSID Class_ID(0x12d45445, 0x3a5779a2)
#define MOTION_PANEL_ROLLUPCFG_CLASSID Class_ID(0xbea1816, 0x50de0291)
#define HIERARCHY_PANEL_ROLLUPCFG_CLASSID Class_ID(0x5d6c08d4, 0x7e5e2c2b)
#define	UTILITY_PANEL_ROLLUPCFG_CLASSID   Class_ID(0x2e256000, 0x6a5b2b34)

/** This class represents the interface class for rollup settings. You can 
 * obtain a pointer to this interface using \code IRollupSettings* 
 * GetIRollupSettings() \endcode. This macro will return 
 * \code (IRollupSettings*)GetCOREInterface(ROLLUP_SETTINGS_INTERFACE)\endcode.
 * \see ICatRegistry, FPStaticInterface */
class IRollupSettings : public FPStaticInterface 
{
	public:
	/** This method initializes the Rollup settings. */
	virtual	void Initialize() = 0;
	/** This method returns a pointer to the category registry. */
	virtual ICatRegistry *GetCatReg()=0;
};

/** This class makes it possible to get and set the category field (order
 * number) for rollups in the RollupOrder.cfg file.
 * Normally a developer would use this class in order to store the order of
 * rollout panels as a result of a custom handled Drag and Drop operation (see
 * IRollupWindow::HandleRollupCallback and IRollupCallback::HandleDrop).
 * First it is necessary to loop through all rollup panels and update the
 * category field in the ICatRegistry like this:
 * \code
 * void TaskRollupManager::UpdateCats()
 * {
 *     for(int i = 0; i < GetNumPanels(); i++)
 *     {
 *         IRollupPanel *panel = GetPanel(GetPanelDlg(i));
 *         Class_ID cid;
 *         SClass_ID sid;
 *         if(GetEditObjClassID(sid, cid))
 *         {
 *             MCHAR title[256];
 *             GetWindowText(panel->GetTitleWnd(), title, 255);
 *             GetIRollupSettings()->GetCatReg()->UpdateCat(sid,cid, title, panel->GetCategory());
 *         }
 *     }
 * }
 * \endcode
 * 
 * After that the ICatRegistry has to be saved with
 * \c GetIRollupSettings()-\>GetCatReg()-\>Save();
 * 
 * In order to reset the rollup order, one has to call 
 * \c ICatRegistry::DeleteList()</b> for a given SClassID and ClassID. The
 * structure of the RollupOrder.cfg file (and thus CatRegistry) can be seen as a
 * list of items that represent the rollup order for each particular group. A
 * group is identified by a SClassID and a ClassID.
 * 
 * In 3dsMax we can have different possibilities to store the order of rollups.
 * In the Create and Modify Panel we want to store the order of rollups for the
 * currently edited object, which can be an object, or a modifier. Thus the
 * ClassID and SClassID of the group is the one of the currently edited
 * object. For the Display panel however, we want the rollups to be the same no
 * matter which object is currently selected. To do this, we create a dummy
 * ClassID which represents the display panel and store the rollup orders under
 * this dummy ClassID. This ClassID is identified by the method 
 * IRollupCallback::GetEditObjClassID. 
 * 
 * \note No ClassDesc has to exist for a dummy ClassID. The RollupOrder.cfg file
 *       will indicate "No ClassName" in case a dummy class id is used.
 *
 * \see IRollupSettings, IRollupWindow, IRollupCallback */
class ICatRegistry : public MaxHeapOperators
{
public:
	/** Destructor. */
	inline virtual ~ICatRegistry() {;}
	/** This method gets the category (order field) for the given SuperClass ID,
	 * ClassID and Rollup Title (and optional Context).
	 * 
	 * \param sid The superclass ID.
	 * \param cid The class ID.
	 * \param title The rollup title.
	 * \param category This is used for error checking; it becomes the default
	 *        value returned by the function if the call fails. So if you pass
	 *        -1000 and get a return value of -1000, that means the entry was
	 *        not found.
	 * \param open Optional pointer to a boolean receiving the open/close state
	 *        of the rollup. If the open/close value was not stored, it does not
	 *        modify the boolean.
	 * \param context An optional context, can be used to specify a special
	 *        rollup ordering based on e.g. specific sub-object selection. 
	 * \return The category (order field) for the given SuperClass ID, ClassID 
	 *        and context, or the default value if there is no stored value for
	 *        the given combination.
	 * \see UpdateCat */
	virtual int GetCat(SClass_ID sid, Class_ID cid, const MCHAR* title, int category, bool* open = nullptr,
			const MCHAR* context = nullptr) const = 0;
	/** This method updates (sets) the category (order field) for the given
	 * SuperClass ID, ClassID and Rollup Title (and optional Context).
	 * 
	 * \param sid The superclass ID.
	 * \param cid The class ID.
	 * \param title The rollup title.
	 * \param category The category.
	 * \param open Optional pointer to the open/close state of the rollup.
	 * \param context Optional context, can be used to specify a special order
	 *        based on e.g. specific sub-object selection. 
	 * \see GetCat */
	virtual void UpdateCat(
			SClass_ID sid, Class_ID cid, const MCHAR* title, int category, const bool* open = nullptr, const MCHAR* context = nullptr) = 0;
	/** This method Saves the category settings in File "UI\RollupOrder.cfg"
	 * \see Load */
	virtual void Save() = 0;
	/** This method Loads the category settings from File "UI\RollupOrder.cfg"
	 * \see Save */
	virtual void Load() = 0;
	/** This method Erases all category settings (in memory only) */
	virtual void EmptyRegistry() = 0;
	/** This method deletes a category list for a given superclass ID, class ID and context.
	 * \param sid The superclass ID.
	 * \param cid The class ID.
	 * \param context Optional context. 
	 * \param deleteContexts If true, all contexts for the given superclass ID
	 *        and class ID are deleted. Only used if context is a nullptr. */
	virtual void DeleteList(SClass_ID sid, Class_ID cid, const MCHAR* context = nullptr, bool deleteContexts = false) = 0;
};

#define ROLLUP_SETTINGS_INTERFACE Interface_ID(0x281a65e8, 0x12db025d)
inline IRollupSettings* GetIRollupSettings() { return (IRollupSettings*)GetCOREInterface(ROLLUP_SETTINGS_INTERFACE); }
