/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: iMenus.h

	 DESCRIPTION: abstract classes for menus

	 CREATED BY: michael malone (mjm)

	 HISTORY: created February 17, 2000

   	 Copyright (c) 2000, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */
#pragma once

#include "maxheap.h"
#include <Geom/color.h>
#include "GetCOREInterface.h"
#include "iFnPub.h"

using ValidityToken = unsigned int;

enum DisplayMethod { DM_NORMAL = 0, DM_STRETCH, DM_FADE, DM_NUM_METHODS };
enum QuadIndex { QUAD_ONE = 0, QUAD_TWO, QUAD_THREE, QUAD_FOUR };


class MenuColors: public MaxHeapOperators
{
public:
	MenuColors() = default;

	// sets values to defaults
	void ResetDefaults()
	{
		*this = MenuColors();
	}

	Color mTitleBarBackgroundColor = Color(.0f, .0f, .0f);
	Color mTitleBarTextColor = Color(.75f, .75f, .75f);
	Color mItemBackgroundColor = Color(.75f, .75f, .75f);
	Color mItemTextColor = Color(.0f, .0f, .0f);
	Color mLastExecutedItemTextColor = Color(.95f, .85f, .0f);
	Color mHighlightedItemBackgroundColor = Color(.95f, .85f, .0f);
	Color mHighlightedItemTextColor = Color(.0f, .0f, .0f);
	Color mBorderColor = Color(.0f, .0f, .0f);
	Color mDisabledShadowColor = Color(.5f, .5f, .5f);
	Color mDisabledHighlightColor = Color(1.0f, 1.0f, 1.0f);
};

inline COLORREF MakeCOLORREF(const Color& c) { return c.toRGB(); }

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/*! \par Description:
This abstract class represents an interface for all general menu settings.
Methods that are marked as internal should not be used.  */
class IMenuSettings: public MaxHeapOperators
{
public:
	/*! \remarks Destructor. */
	virtual ~IMenuSettings() {;}
	// to determine validity of settings
	/*! \remarks This method is used internally.\n\n
	This method checks if a token is valid.
	\par Parameters:
	<b>ValidityToken\& token</b>\n\n
	A reference to a token for which to check its validity.
	\return  TRUE if the token is valid, otherwise FALSE. */
	virtual bool IsTokenValid(const ValidityToken& token) = 0;
	/*! \remarks This method is used internally.\n\n
	This method updates the validity token.
	\par Parameters:
	<b>ValidityToken\& token</b>\n\n
	A reference to a token to update. */
	virtual void UpdateValidityToken(ValidityToken& token) const = 0;

	// sets values to defaults
	/*! \remarks This method will reset the menu settings to their defaults.
	*/
	virtual void ResetDefaults() = 0;

	// sets and gets the border size
	/*! \remarks This method allows you to set the menu border size.
	\par Parameters:
	<b>int borderSz</b>\n\n
	The border size in pixels. */
	virtual void SetBorderSz(int borderSz) = 0;
	/*! \remarks This method returns the menu border size. */
	virtual int GetBorderSz() const = 0;

	// sets and gets the horizontal margin size (in points)
	/*! \remarks This method allows you to set the menu's horizontal margin
	size.
	\par Parameters:
	<b>int horizontalMarginInPoints</b>\n\n
	The horizontal margin size in points. */
	virtual void SetHorizontalMarginInPoints(int horizontalMarginInPoints) = 0;
	/*! \remarks This method returns the menu's horizontal margin size (in
	points). */
	virtual int GetHorizontalMarginInPoints() const = 0;

	// sets and gets the vertical margin size (in points)
	/*! \remarks This method allows you to set the menu's vertical margin
	size.
	\par Parameters:
	<b>int verticalMarginInPoints</b>\n\n
	The vertical margin size in points. */
	virtual void SetVerticalMarginInPoints(int verticalMarginInPoints) = 0;
	/*! \remarks This method returns the menu's vertical margin size (in
	points). */
	virtual int GetVerticalMarginInPoints() const = 0;

	// gets the margins in pixels
	/*! \remarks This method returns the menu's horizontal margin, in pixels.
	\par Parameters:
	<b>HDC hDC</b>\n\n
	A handle to a device context. */
	virtual int GetHorizontalMargin(HDC hDC) const = 0;
	/*! \remarks This method returns the menu's vertical margin, in
	pixels.\n\n

	\par Parameters:
	<b>HDC hDC</b>\n\n
	A handle to a device context. */
	virtual int GetVerticalMargin(HDC hDC) const = 0;

	// sets and gets the item font face
	/*! \remarks This method allows you to set the menu item's font typeface.
	\par Parameters:
	<b>MCHAR* szItemFontFace</b>\n\n
	A string containing the typeface name. */
	virtual void SetItemFontFace(const MCHAR* szItemFontFace) = 0;

	/*! \remarks This method returns the name of the menu item's font
	typeface. */
	virtual const MCHAR* GetItemFontFace() const = 0;

	// sets and gets the title font face
	/*! \remarks This method allows you to set the menu title's font typeface.
	\par Parameters:
	<b>MCHAR* szTitleFontFace</b>\n\n
	A string containing the typeface name. */
	virtual void SetTitleFontFace(const MCHAR* szTitleFontFace) = 0;

	/*! \remarks This method returns the name of the menu title's font
	typeface. */
	virtual const MCHAR* GetTitleFontFace() const = 0;

	// sets and gets the item font size
	/*! \remarks This method allows you to set the menu item's font size.
	\par Parameters:
	<b>int itemFontSize</b>\n\n
	The size of the font, in points. */
	virtual void SetItemFontSize(int itemFontSize) = 0;
	/*! \remarks This method returns the menu item's font size, in points. */
	virtual int GetItemFontSize() const = 0;

	// sets and gets the title font size
	/*! \remarks This method allows you to set the menu title's font size.
	\par Parameters:
	<b>int titleFontSize</b>\n\n
	The size of the font, in points. */
	virtual void SetTitleFontSize(int titleFontSize) = 0;
	/*! \remarks This method returns the menu title's font size, in points. */
	virtual int GetTitleFontSize() const = 0;

	// sets and gets whether menu item's have uniform height
	/*! \remarks This method allows you to set the status of a menu item's
	uniform height flag.
	\par Parameters:
	<b>bool useUniformItemHeight</b>\n\n
	TRUE to set the uniform height flag ON, FALSE to set it to OFF. */
	virtual void SetUseUniformItemHeight(bool useUniformItemHeight) = 0;
	/*! \remarks This method returns TRUE or FALSE if the menu item's uniform
	height flag is set or not set, respectively. */
	virtual bool GetUseUniformItemHeight() const = 0;
	// these overrides are provided for the function publishing system
	/*! \remarks This method allows you to set the status of a menu item's
	uniform height flag. This version of <b>SetUniformItemHeight()</b> is
	provided for the function publishing system.
	\par Parameters:
	<b>BOOL useUniformItemHeight</b>\n\n
	TRUE to set the uniform height flag ON, FALSE to set it to OFF. */
	virtual void SetUseUniformItemHeightBOOL(BOOL useUniformItemHeight) = 0;
	/*! \remarks This method returns TRUE or FALSE if the menu item's uniform
	height flag is set or not set, respectively. This version of
	<b>GetUniformItemHeight()</b> is provided for the function publishing
	system. */
	virtual BOOL GetUseUniformItemHeightBOOL() const = 0;

	// sets and gets the opacity - 0 to 1
	/*! \remarks This method allows you to set the menu's opacity value.
	\par Parameters:
	<b>float opacity</b>\n\n
	The opacity value, ranging from 0.0 - 1.0. */
	virtual void SetOpacity(float opacity) = 0;
	/*! \remarks This method returns the menu's opacity value. */
	virtual float GetOpacity() const = 0;

	// sets and gets the display method
	/*! \remarks This method allows you to set a menu's display method.
	\par Parameters:
	<b>DisplayMethod displayMethod</b>\n\n
	The display method (enum), which is either of the following; <b>DM_NORMAL,
	DM_STRETCH, DM_FADE, DM_NUM_METHODS</b> */
	virtual void SetDisplayMethod(DisplayMethod displayMethod) = 0;
	/*! \remarks This method returns the menu's display method, which is
	either of the following; <b>DM_NORMAL, DM_STRETCH, DM_FADE,
	DM_NUM_METHODS</b> */
	virtual DisplayMethod GetDisplayMethod() const = 0;

	// sets and gets the number of animation steps
	/*! \remarks This method allows you to set the menu's number of animated
	steps for the 'growing' effect.
	\par Parameters:
	<b>unsigned int steps</b>\n\n
	The number of steps. */
	virtual void SetAnimatedSteps(unsigned int steps) = 0;
	/*! \remarks This method returns the menu's number of animated steps used
	for the 'growing' effect. */
	virtual unsigned int GetAnimatedSteps() const = 0;

	// sets and gets the duration of an animation step (milliseconds)
	/*! \remarks This method allows you to set the menu's animated step time.
	\par Parameters:
	<b>unsigned int ms</b>\n\n
	The animated step time, in milliseconds. */
	virtual void SetAnimatedStepTime(unsigned int ms) = 0;
	/*! \remarks This method returns the menu's animated step time, in
	milliseconds. */
	virtual unsigned int GetAnimatedStepTime() const = 0;

	// sets and gets the delay before a submenu is displayed (milliseconds)
	/*! \remarks This method allows you to set the delay before a submenu is
	displayed.
	\par Parameters:
	<b>unsigned int ms</b>\n\n
	The delay, in milliseconds. */
	virtual void SetSubMenuPauseTime(unsigned int ms) = 0;
	/*! \remarks This method returns the delay before a submenu is displayed,
	in milliseconds. */
	virtual unsigned int GetSubMenuPauseTime() const = 0;

	// sets and gets whether to use the menu's last executed item (when user clicks in title bar)
	/*! \remarks This method allows you to set the "last executed item" flag
	which determines whether to use the menu's last executed item when the user
	clicks on the menu's titlebar.
	\par Parameters:
	<b>bool useLastExecutedItem</b>\n\n
	TRUE to turn ON the flag, FALSE to turn the flag off. */
	virtual void SetUseLastExecutedItem(bool useLastExecutedItem) = 0;
	/*! \remarks This method returns whether the "last executed item" flag is
	set (TRUE) or not set (FALSE). The flag determines whether to use the
	menu's last executed item when the user clicks on the menu's titlebar. */
	virtual bool GetUseLastExecutedItem() const = 0;
	// these overrides are provided for the function publishing system
	/*! \remarks This method allows you to set the "last executed item" flag
	which determines whether to use the menu's last executed item when the user
	clicks on the menu's titlebar. This version of
	<b>SetUseLastExecutedItem()</b> is provided for the function publishing
	system.\n\n

	\par Parameters:
	<b>BOOL useLastExecutedItem</b>\n\n
	TRUE to turn ON the flag, FALSE to turn the flag off. */
	virtual void SetUseLastExecutedItemBOOL(BOOL useLastExecutedItem) = 0;
	/*! \remarks This method returns whether the "last executed item" flag is
	set (TRUE) or not set (FALSE). The flag determines whether to use the
	menu's last executed item when the user clicks on the menu's titlebar. This
	version of <b>GetUseLastExecutedItem()</b> is provided for the function
	publishing system. */
	virtual BOOL GetUseLastExecutedItemBOOL() const = 0;

	// sets and gets whether the menu is repositioned when near the edge of the screen
	/*! \remarks This method allows you to set the flag which controls and
	determines whether the menu is repositioned when near the edge of the
	screen.
	\par Parameters:
	<b>bool repositionWhenClipped</b>\n\n
	TRUE to turn repositioning ON, FALSE to turn it OFF. */
	virtual void SetRepositionWhenClipped(bool repositionWhenClipped) = 0;
	/*! \remarks This method returns the status of the flag which controls and
	determines whether the menu is repositioned when near the edge of the
	screen.
	\return  TRUE if the flag is ON, otherwise FALSE. */
	virtual bool GetRepositionWhenClipped() const = 0;
	// these overrides are provided for the function publishing system
	/*! \remarks This method allows you to set the flag which controls and
	determines whether the menu is repositioned when near the edge of the
	screen. This version of <b>SetRepositionWhenClipped()</b> is provided for
	the function publishing system.
	\par Parameters:
	<b>BOOL repositionWhenClipped</b>\n\n
	TRUE to turn repositioning ON, FALSE to turn it OFF. */
	virtual void SetRepositionWhenClippedBOOL(BOOL repositionWhenClipped) = 0;
	/*! \remarks This method returns the status of the flag which controls and
	determines whether the menu is repositioned when near the edge of the
	screen. This version of <b>GetRepositionWhenClipped()</b> is provided for
	the function publishing system.
	\return  TRUE if the flag is ON, otherwise FALSE. */
	virtual BOOL GetRepositionWhenClippedBOOL() const = 0;

	// sets and gets whether the menu should remove redundant separators
	/*! \remarks This method allows you to set the flag which controls and
	determines whether the menu should remove redundant separators.
	\par Parameters:
	<b>bool removeRedundantSeparators</b>\n\n
	TRUE to turn the flag ON, FALSE to turn it OFF. */
	virtual void SetRemoveRedundantSeparators(bool removeRedundantSeparators) = 0;
	/*! \remarks This method returns the status of the flag which controls and
	determines whether the menu should remove redundant separators.
	\return  TRUE if the flag is ON, otherwise FALSE. */
	virtual bool GetRemoveRedundantSeparators() const = 0;
	// these overrides are provided for the function publishing system
	/*! \remarks This method allows you to set the flag which controls and
	determines whether the menu should remove redundant separators. This
	version of <b>SetRemoveRedundantSeparators()</b> is provided for the
	function publishing system.
	\par Parameters:
	<b>BOOL removeRedundantSeparators</b>\n\n
	TRUE to turn the flag ON, FALSE to turn it OFF. */
	virtual void SetRemoveRedundantSeparatorsBOOL(BOOL removeRedundantSeparators) = 0;
	/*! \remarks This method returns the status of the flag which controls and
	determines whether the menu should remove redundant separators. This
	version of <b>GetRemoveRedundantSeparators()</b> is provided for the
	function publishing system.
	\return  TRUE if the flag is ON, otherwise FALSE. */
	virtual BOOL GetRemoveRedundantSeparatorsBOOL() const = 0;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/*! \sa Class IMenuSettings,  Class MenuColors\n\n
\par Description:
This abstract class represents an interface for quad menu settings. The methods
contained in this class allow you to access and control all quad menu related
settings and configuration parameters.  */
class IQuadMenuSettings : public IMenuSettings, public FPStaticInterface
{
public:
// function IDs 
	enum
	{
		// from IMenuSettings
		fnIdResetDefaults,
		fnIdSetBorderSize,
		fnIdGetBorderSize,
		fnIdSetHorizontalMarginInPoints,
		fnIdGetHorizontalMarginInPoints,
		fnIdSetVerticalMarginInPoints,
		fnIdGetVerticalMarginInPoints,
		fnIdSetItemFontFace,
		fnIdGetItemFontFace,
		fnIdSetTitleFontFace,
		fnIdGetTitleFontFace,
		fnIdSetItemFontSize,
		fnIdGetItemFontSize,
		fnIdSetTitleFontSize,
		fnIdGetTitleFontSize,
		fnIdSetUseUniformItemHeight,
		fnIdGetUseUniformItemHeight,
		fnIdSetOpacity,
		fnIdGetOpacity,
		fnIdSetDisplayMethod,
		fnIdGetDisplayMethod,
		fnIdSetAnimatedSteps,
		fnIdGetAnimatedSteps,
		fnIdSetAnimatedStepTime,
		fnIdGetAnimatedStepTime,
		fnIdSetSubMenuPauseTime,
		fnIdGetSubMenuPauseTime,
		fnIdSetUseLastExecutedItem,
		fnIdGetUseLastExecutedItem,
		fnIdSetRepositionWhenClipped,
		fnIdGetRepositionWhenClipped,
		fnIdSetRemoveRedundantSeparators,
		fnIdGetRemoveRedundantSeparators,

		// from IQuadMenuSettings
		fnIdSetFirstQuadDisplayed,
		fnIdGetFirstQuadDisplayed,
		fnIdSetUseUniformQuadWidth,
		fnIdGetUseUniformQuadWidth,
		fnIdSetMoveCursorOnReposition,
		fnIdGetMoveCursorOnReposition,
		fnIdSetReturnCursorAfterReposition,
		fnIdGetReturnCursorAfterReposition,
		fnIdSetInitialCursorLocInBox_0to1,
		fnIdGetInitialCursorLocXInBox_0to1,
		fnIdGetInitialCursorLocYInBox_0to1,

		fnIdSetTitleBarBackgroundColor,
		fnIdGetTitleBarBackgroundColor,
		fnIdSetTitleBarTextColor,
		fnIdGetTitleBarTextColor,
		fnIdSetItemBackgroundColor,
		fnIdGetItemBackgroundColor,
		fnIdSetItemTextColor,
		fnIdGetItemTextColor,
		fnIdSetLastExecutedItemTextColor,
		fnIdGetLastExecutedItemTextColor,
		fnIdSetHighlightedItemBackgroundColor,
		fnIdGetHighlightedItemBackgroundColor,
		fnIdSetHighlightedItemTextColor,
		fnIdGetHighlightedItemTextColor,
		fnIdSetBorderColor,
		fnIdGetBorderColor,
		fnIdSetDisabledShadowColor,
		fnIdGetDisabledShadowColor,
		fnIdSetDisabledHighlightColor,
		fnIdGetDisabledHighlightColor,
};

	// sets and gets the first quadrant displayed
	/*! \remarks This method allows you to set the first quad which will be
	displayed when a quad menu pops up.
	\par Parameters:
	<b>QuadIndex firstQuadDisplayed</b>\n\n
	The quad index, one of the following; <b>QUAD_ONE</b>, <b>QUAD_TWO</b>,
	<b>QUAD_THREE</b>, or <b>QUAD_FOUR</b>. */
	virtual void SetFirstQuadDisplayed(QuadIndex firstQuadDisplayed) = 0;
	/*! \remarks This method returns the index of the first quad which will be
	displayed.
	\return  The quad index, one of the following; <b>QUAD_ONE</b>,
	<b>QUAD_TWO</b>, <b>QUAD_THREE</b>, or <b>QUAD_FOUR</b>. */
	virtual QuadIndex GetFirstQuadDisplayed() const = 0;

	// sets and gets whether the quadrants have uniform width
	/*! \remarks This method allows you to set whether the quad menu has a
	uniform width.
	\par Parameters:
	<b>bool useUniformQuadWidth</b>\n\n
	TRUE to set the uniform width, FALSE to set it to non-uniform. */
	virtual void SetUseUniformQuadWidth(bool useUniformQuadWidth) = 0;
	/*! \remarks This method returns the status of the uniform width flag for
	the quad menu. TRUE if the quad menu has been set to use uniform width,
	otherwise FALSE. */
	virtual bool GetUseUniformQuadWidth() const = 0;
	// these overrides are provided for the function publishing system
	/*! \remarks This method allows you to set whether the quad menu has a
	uniform width. This version of <b>SetUseUniformQuadWidth()</b> is provided
	for the function publishing system.
	\par Parameters:
	<b>BOOL useUniformQuadWidth</b>\n\n
	TRUE to set the uniform width, FALSE to set it to non-uniform. */
	virtual void SetUseUniformQuadWidthBOOL(BOOL useUniformQuadWidth) = 0;
	/*! \remarks This method returns the status of the uniform width flag for
	the quad menu. TRUE if the quad menu has been set to use uniform width,
	otherwise FALSE. This version of <b>GetUseUniformQuadWidth()</b> is
	provided for the function publishing system. */
	virtual BOOL GetUseUniformQuadWidthBOOL() const = 0;

	// sets and gets whether the cursor moves when the quad menu is repositioned because of clipping the edge of the screen
	/*! \remarks This method allows you to set whether the cursor moves when
	the quad menu is repositioned because of clipping the edge of the screen.
	\par Parameters:
	<b>bool moveCursorOnReposition</b>\n\n
	TRUE to move the cursor, otherwise FALSE. */
	virtual void SetMoveCursorOnReposition(bool moveCursorOnReposition) = 0;
	/*! \remarks This method returns TRUE if the cursor moves when the quad
	menu is repositioned because of clipping the edge of the screen, otherwise
	FALSE. */
	virtual bool GetMoveCursorOnReposition() const = 0;
	// these overrides are provided for the function publishing system
	/*! \remarks This method allows you to set whether the cursor moves when
	the quad menu is repositioned because of clipping the edge of the screen.
	This version of <b>SetMoveCursorOnReposition()</b> is provided for the
	function publishing system.
	\par Parameters:
	<b>BOOL moveCursorOnReposition</b>\n\n
	TRUE to move the cursor, otherwise FALSE. */
	virtual void SetMoveCursorOnRepositionBOOL(BOOL moveCursorOnReposition) = 0;
	/*! \remarks This method returns TRUE if the cursor moves when the quad
	menu is repositioned because of clipping the edge of the screen, otherwise
	FALSE. This version of <b>GetMoveCursorOnReposition()</b> is provided for
	the function publishing system. */
	virtual BOOL GetMoveCursorOnRepositionBOOL() const = 0;

	// sets and gets whether the cursor is moved the opposite distance that it was automatically moved when the quad menu is repositioned because of clipping the edge of the screen
	/*! \remarks This method allows you to set whether the cursor is moved the
	opposite distance that it was automatically moved when the quad menu is
	repositioned because of clipping the edge of the screen.
	\par Parameters:
	<b>bool returnCursorAfterReposition</b>\n\n
	TRUE to set the flag, otherwise FALSE. */
	virtual void SetReturnCursorAfterReposition(bool returnCursorAfterReposition) = 0;
	/*! \remarks This method returns TRUE if the cursor is moved the opposite
	distance that it was automatically moved when the quad menu is repositioned
	because of clipping the edge of the screen, otherwise FALSE. */
	virtual bool GetReturnCursorAfterReposition() const = 0;
	// these overrides are provided for the function publishing system
	/*! \remarks This method allows you to set whether the cursor is moved the
	opposite distance that it was automatically moved when the quad menu is
	repositioned because of clipping the edge of the screen. This version of
	<b>GetReturnCursorAfterReposition()</b> is provided for the function
	publishing system.
	\par Parameters:
	<b>BOOL returnCursorAfterReposition</b>\n\n
	TRUE to set the flag, otherwise FALSE. */
	virtual void SetReturnCursorAfterRepositionBOOL(BOOL returnCursorAfterReposition) = 0;
	/*! \remarks This method returns TRUE if the cursor is moved the opposite
	distance that it was automatically moved when the quad menu is repositioned
	because of clipping the edge of the screen, otherwise FALSE. This version
	of <b>GetReturnCursorAfterReposition()</b> is provided for the function
	publishing system. */
	virtual BOOL GetReturnCursorAfterRepositionBOOL() const = 0;

	// sets and gets the initial location of the cursor in the center box - as a ratio (0 to 1) of box size
	/*! \remarks This method allows you to set the initial location of the
	cursor in the center quad box.
	\par Parameters:
	<b>float x, float y</b>\n\n
	The location of the cursor, as a ratio of the box size, between 0.0 and
	1.0. */
	virtual void SetCursorLocInBox_0to1(float x, float y) = 0;
	/*! \remarks This method returns the initial x location of the cursor in
	the center quad box, as a ratio of the box size, between 0.0 and 1.0. */
	virtual float GetCursorLocXInBox_0to1() const = 0;
	/*! \remarks This method returns the initial y location of the cursor in
	the center quad box, as a ratio of the box size, between 0.0 and 1.0. */
	virtual float GetCursorLocYInBox_0to1() const = 0;


	// gets the color array for a specific quad (numbered 1 through 4)
	/*! \remarks This method returns the color array for a specific quad.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad to obtain the color array for, (numbered 1 through 4). */
	virtual const MenuColors *GetMenuColors(int quadNum) const = 0;

	// sets and gets the title bar background color for a specific quad (numbered 1 through 4)
	/*! \remarks This method allows you to set the title bar background color
	for a specific quad.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4).\n\n
	<b>Color\& color</b>\n\n
	The color to set. */
	virtual void SetTitleBarBackgroundColor(int quadNum, const Color& color) = 0;
	/*! \remarks This method returns the title bar background color of a
	specific quad. This method returns the color as a <b>Color</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual const Color& GetTitleBarBackgroundColor(int quadNum) const = 0;
	/*! \remarks This method returns the title bar background color of a
	specific quad. This method returns the color as a <b>COLORREF</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual COLORREF GetTitleBarBackgroundColorRef(int quadNum) const = 0;

	// sets and gets the title bar text color for a specific quad (numbered 1 through 4)
	/*! \remarks This method allows you to set the title bar text color for a
	specific quad.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4).\n\n
	<b>Color\& color</b>\n\n
	The color to set. */
	virtual void SetTitleBarTextColor(int quadNum, const Color& color) = 0;
	/*! \remarks This method returns the title bar text color of a specific
	quad. This method returns the color as a <b>Color</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual const Color& GetTitleBarTextColor(int quadNum) const = 0;
	/*! \remarks This method returns the title bar text color of a specific
	quad. This method returns the color as a <b>COLORREF</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual COLORREF GetTitleBarTextColorRef(int quadNum) const = 0;

	// sets and gets the item background color for a specific quad (numbered 1 through 4)
	/*! \remarks This method allows you to set the item background color for a
	specific quad.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4).\n\n
	<b>Color\& color</b>\n\n
	The color to set. */
	virtual void SetItemBackgroundColor(int quadNum, const Color& color) = 0;
	/*! \remarks This method returns the item background color of a specific
	quad. This method returns the color as a <b>Color</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual const Color& GetItemBackgroundColor(int quadNum) const = 0;
	/*! \remarks This method returns the item background color of a specific
	quad. This method returns the color as a <b>COLORREF</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual COLORREF GetItemBackgroundColorRef(int quadNum) const = 0;

	// sets and gets the item text color for a specific quad (numbered 1 through 4)
	/*! \remarks This method allows you to set the item text color for a
	specific quad.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4).\n\n
	<b>Color\& color</b>\n\n
	The color to set. */
	virtual void SetItemTextColor(int quadNum, const Color& color) = 0;
	/*! \remarks This method returns the item text color of a specific quad.
	This method returns the color as a <b>Color</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual const Color& GetItemTextColor(int quadNum) const = 0;
	/*! \remarks This method returns the item text color of a specific quad.
	This method returns the color as a <b>COLORREF</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual COLORREF GetItemTextColorRef(int quadNum) const = 0;

	// sets and gets the last executed item text color for a specific quad (numbered 1 through 4)
	/*! \remarks This method allows you to set the last executed item text
	color for a specific quad.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4).\n\n
	<b>Color\& color</b>\n\n
	The color to set. */
	virtual void SetLastExecutedItemTextColor(int quadNum, const Color& color) = 0;
	/*! \remarks This method returns the last executed item text color of a
	specific quad. This method returns the color as a <b>Color</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual const Color& GetLastExecutedItemTextColor(int quadNum) const = 0;
	/*! \remarks This method returns the last executed item text color of a
	specific quad. This method returns the color as a <b>COLORREF</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual COLORREF GetLastExecutedItemTextColorRef(int quadNum) const = 0;

	// sets and gets the highlighted item background color for a specific quad (numbered 1 through 4)
	/*! \remarks This method allows you to set the highlighted item background
	color for a specific quad.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4).\n\n
	<b>Color\& color</b>\n\n
	The color to set. */
	virtual void SetHighlightedItemBackgroundColor(int quadNum, const Color& color) = 0;
	/*! \remarks This method returns the highlighted item background color of
	a specific quad. This method returns the color as a <b>Color</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual const Color& GetHighlightedItemBackgroundColor(int quadNum) const = 0;
	/*! \remarks This method returns the highlighted item background color of
	a specific quad. This method returns the color as a <b>COLORREF</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual COLORREF GetHighlightedItemBackgroundColorRef(int quadNum) const = 0;

	// sets and gets the highlighted item text color for a specific quad (numbered 1 through 4)
	/*! \remarks This method allows you to set the highlighted item text color
	for a specific quad.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4).\n\n
	<b>Color\& color</b>\n\n
	The color to set. */
	virtual void SetHighlightedItemTextColor(int quadNum, const Color& color) = 0;
	/*! \remarks This method returns the highlighted item text color of a
	specific quad. This method returns the color as a <b>Color</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual const Color& GetHighlightedItemTextColor(int quadNum) const = 0;
	/*! \remarks This method returns the highlighted item text color of a
	specific quad. This method returns the color as a <b>COLORREF</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual COLORREF GetHighlightedItemTextColorRef(int quadNum) const = 0;

	// sets and gets the border color for a specific quad (numbered 1 through 4)
	/*! \remarks This method allows you to set the border color for a specific
	quad.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4).\n\n
	<b>Color\& color</b>\n\n
	The color to set. */
	virtual void SetBorderColor(int quadNum, const Color& color) = 0;
	/*! \remarks This method returns the border color of a specific quad. This
	method returns the color as a <b>Color</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual const Color& GetBorderColor(int quadNum) const = 0;
	/*! \remarks This method returns the border color of a specific quad. This
	method returns the color as a <b>COLORREF</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual COLORREF GetBorderColorRef(int quadNum) const = 0;

	// sets and gets the disabled shadow color for a specific quad (numbered 1 through 4)
	/*! \remarks This method allows you to set the disabled shadow color for a
	specific quad.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4).\n\n
	<b>Color\& color</b>\n\n
	The color to set. */
	virtual void SetDisabledShadowColor(int quadNum, const Color& color) = 0;
	/*! \remarks This method returns the disabled shadow color of a specific
	quad. This method returns the color as a <b>Color</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual const Color& GetDisabledShadowColor(int quadNum) const = 0;
	/*! \remarks This method returns the disabled shadow color of a specific
	quad. This method returns the color as a <b>COLORREF</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual COLORREF GetDisabledShadowColorRef(int quadNum) const = 0;

	// sets and gets the disabled highlight color for a specific quad (numbered 1 through 4)
	/*! \remarks This method allows you to set the disabled highlight color
	for a specific quad.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4).\n\n
	<b>Color\& color</b>\n\n
	The color to set. */
	virtual void SetDisabledHighlightColor(int quadNum, const Color& color) = 0;
	/*! \remarks This method returns the disabled highlight color of a
	specific quad. This method returns the color as a <b>Color</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual const Color& GetDisabledHighlightColor(int quadNum) const = 0;
	/*! \remarks This method returns the disabled highlight color of a
	specific quad. This method returns the color as a <b>COLORREF</b>.
	\par Parameters:
	<b>int quadNum</b>\n\n
	The quad (numbered 1 through 4). */
	virtual COLORREF GetDisabledHighlightColorRef(int quadNum) const = 0;
};


#define MENU_SETTINGS Interface_ID(0x31561ddb, 0x1a2f4619)
inline IQuadMenuSettings* GetQuadSettings() { return (IQuadMenuSettings*)GetCOREInterface(MENU_SETTINGS); }
