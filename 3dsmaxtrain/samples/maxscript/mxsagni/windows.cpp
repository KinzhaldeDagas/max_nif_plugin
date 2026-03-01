/**********************************************************************
*<
FILE: windows.cpp

DESCRIPTION: 

CREATED BY: Larry Minton

HISTORY: Created 15 April 2007

*>	Copyright (c) 2007, All Rights Reserved.
**********************************************************************/

#include <maxscript/maxscript.h>
#include <maxscript/foundation/numbers.h>
#include <maxscript/foundation/3dmath.h>		// Point2Value
#include <maxscript/foundation/strings.h>
#include <maxscript/maxwrapper/bitmaps.h>
#include <maxscript/UI/rollouts.h>

#include "MXSAgni.h"
#include "agnidefs.h"
#include <winutil.h>
#include <QtCore/qcoreapplication.h>

#define INITROUS_DISPLAY_MANAGER_INTERNAL_INTERFACE_ID Interface_ID(0x73418f88, 0x52c04a20)

#ifdef ScripterExport
#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

using namespace MAXScript;

#include <maxscript\macros\define_instantiation_functions.h>
#include "extclass.h"

extern Bitmap* CreateBitmapFromBInfo(void **ptr, const int cx, const int cy, bool captureAlpha);
extern void DeGammaCorrectBitmap(Bitmap* map);

// ============================================================================

#include <maxscript\macros\define_external_functions.h>
#	include "namedefs.h"

#include <maxscript\macros\define_instantiation_functions.h>
#	include "windows_wraps.h"
// -----------------------------------------------------------------------------------------

namespace
{
	//This function calls Nitrous to let the progressive rendering go forward by one iteration
	bool ProgressiveRendering(FPStaticInterface& nitrousGraphicsManager)
	{
		static const short progressiveRenderingID = 3;//Is FPDisplayManagerInternal::_ProgressiveRendering

		FPValue result;
		nitrousGraphicsManager.Invoke(progressiveRenderingID, result);
		return result.b;
	}
};

struct EnumProcArg
{
	Array* returnArray; // will contain the array to store results it
	bool checkParent; // if true, check window parent against parent member value below
	HWND parent; // parent window value to check against
	const TCHAR* matchString; // string to test window text against
	EnumProcArg(Array* returnArray, bool checkParent, HWND parent, const TCHAR* matchString = nullptr) 
		: returnArray(returnArray), checkParent(checkParent), parent(parent), matchString(matchString) {}
};

static const int CollectWindowData_DataSize = 8;

// function for collecting data for a specified window
static void CollectWindowData (HWND hWnd, Array *winData)
{
	TCHAR className[256] = {};
	MSTR windowText;
	int bufSize = _countof(className);

	// 1: HWND
	winData->append(IntegerPtr::intern((INT_PTR)hWnd));  // 1
	HWND ancestor = GetAncestor(hWnd,GA_PARENT);
	// 2: the parent HWND. This does not include the owner
	winData->append(IntegerPtr::intern((INT_PTR)ancestor)); // 2
	HWND par = GetParent(hWnd);
	// 3: the parent or owner HWND. If the window is a child window, the value is a handle to the parent window. 
	// If the window is a top-level window, the value is a handle to the owner window.
	winData->append(IntegerPtr::intern((INT_PTR)par)); // 3
	GetClassName(hWnd, className, bufSize);
	// 4: class name as a string - limit of 255 characters
	winData->append(new String(className)); // 4
	GetWindowText(hWnd, windowText);
	// 5: window text as a string
	winData->append(new String(windowText)); // 5
	HWND owner = GetWindow(hWnd,GW_OWNER);
	// 6: the owner's HWND 
	winData->append(IntegerPtr::intern((INT_PTR)owner)); // 6
	HWND root = GetAncestor(hWnd,GA_ROOT);
	// 7: root window HWND. The root window as determined by walking the chain of parent windows
	winData->append(IntegerPtr::intern((INT_PTR)root)); // 7
	HWND rootowner = GetAncestor(hWnd,GA_ROOTOWNER);
	// 8: owned root window HWND. The owned root window as determined by walking the chain of parent and owner 
	// windows returned by GetParent
	winData->append(IntegerPtr::intern((INT_PTR)rootowner)); // 8
}

static HWND Value_to_HWND(Value* val, const TCHAR* fn_name, bool methodAllowsRolloutControl = false)
{
	HWND hWnd = nullptr;
	if (val == n_max)
		hWnd = MAXScript_interface->GetMAXHWnd();
	else if (val == &unsupplied)
		hWnd = nullptr;
	else if (is_rolloutfloater(val))
	{
		RolloutFloater* rof = (RolloutFloater*)val;
		if (rof->window)
			hWnd = rof->window;
	}
	else if (is_rollout(val))
	{
		Rollout* ro = (Rollout*)val;
		if (ro->page)
			hWnd = ro->page;
	}
	else if (Number::is_integer_number(val))
	{
		hWnd = (HWND)val->to_intptr();
	}
	else
	{
		TSTR errMsg;
		TSTR fmt = MaxSDK::GetResourceStringAsMSTR(
				methodAllowsRolloutControl ? IDS_WINDOWS_METHOD_ARG_TYPE_ERROR_1 : IDS_WINDOWS_METHOD_ARG_TYPE_ERROR_2);
		errMsg.printf(fmt, fn_name);
		throw RuntimeError(errMsg, val);
	}

	return hWnd;
}

Value* getData_HWND_cf(Value** arg_list, int count)
{
	// windows.getHWNDData {<RolloutFloater>|<Rollout>|<int_HWND>|#max} 
	check_arg_count(windows.getHWNDData, 1, count);
	HWND hWnd = Value_to_HWND(arg_list[0], _T("windows.getHWNDData"));
	if (hWnd == nullptr)
		hWnd = GetDesktopWindow();

	if (IsWindow(hWnd)) 
	{
		MAXScript_TLS* _tls = (MAXScript_TLS*)TlsGetValue(thread_locals_index);
		one_typed_value_local_tls(Array* result);
		vl.result = new Array (CollectWindowData_DataSize);
		CollectWindowData(hWnd, vl.result);
		return_value_tls(vl.result);
	}
	else
		return &undefined;
}

static BOOL CALLBACK CollectorEnumChildProc(HWND hWnd, LPARAM lParam)
{
	EnumProcArg *enumProcArg = (EnumProcArg*)lParam;

	HWND par = GetParent(hWnd);
	if (enumProcArg->checkParent && (par != enumProcArg->parent)) 
		return TRUE;

	Array *winData = new Array(CollectWindowData_DataSize);
	enumProcArg->returnArray->append(winData);
	CollectWindowData(hWnd, winData);

	return TRUE;
}

Value* getChildren_HWND_cf(Value** arg_list, int count)
{
	// windows.getChildrenHWND {<RolloutFloater>|<Rollout>|<int_HWND>|#max} parent:{<RolloutFloater>|<Rollout>|<int_HWND>|#max} 
	check_arg_count_with_keys(windows.getChildrenHWND, 1, count);
	HWND hWnd = Value_to_HWND(arg_list[0], _T("windows.getChildrenHWND"));

	Value* tmp = key_arg(parent);
	HWND parent_hwnd = Value_to_HWND(tmp, _T("windows.getChildrenHWND parent:"));
	bool checkParent = tmp != &unsupplied;
	if (checkParent && parent_hwnd == nullptr)
		parent_hwnd = GetDesktopWindow();

	if (hWnd == nullptr || IsWindow(hWnd)) 
	{
		MAXScript_TLS* _tls = (MAXScript_TLS*)TlsGetValue(thread_locals_index);
		one_typed_value_local_tls(Array* result);
		vl.result = new Array (0);
		EnumProcArg enumProcArg(vl.result, checkParent, parent_hwnd);
		EnumChildWindows(hWnd, CollectorEnumChildProc, (LPARAM)&enumProcArg);
		return_value_tls(vl.result);
	}
	else
		return &undefined;
}

static BOOL CALLBACK MatchEnumChildProc(HWND hWnd, LPARAM lParam)
{
	EnumProcArg *enumProcArg = (EnumProcArg*)lParam;

	HWND par = GetParent(hWnd);
	if (enumProcArg->checkParent && (par != enumProcArg->parent)) 
		return TRUE;

	MSTR windowText;
	GetWindowText(hWnd, windowText);

	if (_tcsicmp(windowText.data(), enumProcArg->matchString) == 0)
	{
		CollectWindowData(hWnd, enumProcArg->returnArray);
		return FALSE;
	}

	return TRUE;
}

Value* getChild_HWND_cf(Value** arg_list, int count)
{
	// windows.getChildHWND {<RolloutFloater>|<Rollout>|<int_HWND>|#max} <string> parent:{<RolloutFloater>|<Rollout>|<int_HWND>|#max}
	check_arg_count_with_keys(windows.getChildHWND, 2, count);
	HWND hWnd = Value_to_HWND(arg_list[0], _T("windows.getChildHWND"));

	Value *tmp = key_arg(parent);
	HWND parent_hwnd = Value_to_HWND(tmp, _T("windows.getChildHWND parent:"));
	bool checkParent = tmp != &unsupplied;
	if (checkParent && parent_hwnd == nullptr)
		parent_hwnd = GetDesktopWindow();

	if (hWnd == nullptr || IsWindow(hWnd)) 
	{
		MAXScript_TLS* _tls = (MAXScript_TLS*)TlsGetValue(thread_locals_index);
		one_typed_value_local_tls(Array* result);
		vl.result = new Array (0);
		EnumProcArg enumProcArg(vl.result, checkParent, parent_hwnd, arg_list[1]->to_string());
		BOOL notFound = EnumChildWindows(hWnd, MatchEnumChildProc, (LPARAM)&enumProcArg);
		if (notFound)
			return_value_tls(&undefined);
		else
			return_value_tls(vl.result);
	}
	else
		return &undefined;
}

Value* getMAX_HWND_cf(Value** arg_list, int count)
{
	// windows.getMAXHWND() 
	check_arg_count(windows.getMAXHWND, 0, count);
	return_value (IntegerPtr::intern((INT_PTR)MAXScript_interface->GetMAXHWnd()));
}

Value* getDesktop_HWND_cf(Value** arg_list, int count)
{
	// windows.getDesktopHWND() 
	check_arg_count(windows.getDesktopHWND, 0, count);
	return_value (IntegerPtr::intern((INT_PTR)GetDesktopWindow()));
}

Value* send_message_cf(Value** arg_list, int count)
{
	// windows.sendMessage {<RolloutFloater>|<Rollout>|<int_HWND>|#max} <int message> <int messageParameter1> <int messageParameter2>
	check_arg_count(windows.sendMessage, 4, count);

	MAXScript::ValidateCanRunMAXScriptSystemCommand(_T("windows.sendMessage"), arg_list, count);

	HWND hWnd = Value_to_HWND(arg_list[0], _T("windows.sendMessage"));
	HRESULT hRes = SendMessage(
		hWnd,
		(UINT)arg_list[1]->to_int(),
		(WPARAM)arg_list[2]->to_intptr(),
		(LPARAM)arg_list[3]->to_intptr());
	return_value (Integer::intern(hRes));
}

Value* post_message_cf(Value** arg_list, int count)
{
	// windows.postMessage {<RolloutFloater>|<Rollout>|<int_HWND>|#max} <int message> <int messageParameter1> <int messageParameter2>
	check_arg_count(windows.postMessage, 4, count);

	MAXScript::ValidateCanRunMAXScriptSystemCommand(_T("windows.postMessage"), arg_list, count);

	HWND hWnd = Value_to_HWND(arg_list[0], _T("windows.postMessage"));
	BOOL res = PostMessage(
		hWnd,
		(UINT)arg_list[1]->to_int(),
		(WPARAM)arg_list[2]->to_intptr(),
		(LPARAM)arg_list[3]->to_intptr());
	return bool_result(res);
}

static ULONGLONG lastPeek_processPostedMessages = 0;

Value* process_messages_cf(Value** arg_list, int count)
{
	static FPStaticInterface* pINitrousManager = nullptr;

	// windows.processPostedMessages throttle:<bool> sendQtPostedEvents:<bool> doProgressiveRendering:<bool>
	check_arg_count_with_keys(windows.processPostedMessages, 0, count);
	Value* dmy;
	bool throttle = bool_key_arg(throttle, dmy, false);
	bool sendQtPostedEvents = bool_key_arg(sendQtPostedEvents, dmy, true);
	bool doProgressiveRendering = bool_key_arg(doProgressiveRendering, dmy, true);

	if (throttle)
	{
		if ((GetTickCount64() - lastPeek_processPostedMessages) < 1000)
			return &ok;
	}

	//Init pINitrousManager if needed
	if (nullptr == pINitrousManager){
		pINitrousManager = (FPStaticInterface*)GetCOREInterface(INITROUS_DISPLAY_MANAGER_INTERNAL_INTERFACE_ID);
	}

	MSG wmsg;
	while (PeekMessage(&wmsg, nullptr, 0, 0, PM_REMOVE))
	{
		lastPeek_processPostedMessages = GetTickCount64();
		MAXScript_interface->TranslateAndDispatchMAXMessage(wmsg);
		if (GetCOREInterface7()->QuitingApp())
		{
			throw SignalException();
		}
	}

	// There might be queued Qt events that need to be included.
	if (sendQtPostedEvents)
		QCoreApplication::sendPostedEvents();

	if (doProgressiveRendering)
		ProgressiveRendering(*pINitrousManager);//Make Progressive rendering go forward by one iteration.

	return &ok;
}

static ULONGLONG lastPeek_peekMessage = 0;

Value* peek_Message_cf(Value** arg_list, int count)
{
	// windows.peekMessage throttle:<bool>
	check_arg_count_with_keys(windows.peekMessage, 0, count);
	Value* dmy;
	bool throttle = bool_key_arg(throttle, dmy, true);
	if (throttle)
	{
		ULONGLONG now = GetTickCount64();
		if ((now - lastPeek_peekMessage) < 1000 || (now - lastPeek_processPostedMessages) < 1000)
			return &ok;
	}
	MSG msg;
	PeekMessage(&msg, (HWND)NULL, 0, 0, PM_NOREMOVE);
	lastPeek_peekMessage = GetTickCount64();
	return &ok;
}

Value* getParent_HWND_cf(Value** arg_list, int count)
{
	// windows.getParentHWND {<RolloutFloater>|<Rollout>|<int_HWND>|#max}
	check_arg_count(windows.getParentHWND, 1, count);
	HWND hWnd = Value_to_HWND(arg_list[0], _T("windows.getParentHWND"));
	return_value (IntegerPtr::intern((INT_PTR)::GetParent(hWnd)));
}

Value* set_window_pos_cf(Value** arg_list, int count)
{
	// windows.setWindowPos {<RolloutFloater>|<Rollout>|<int_HWND>|#max} <int x> <int y> <int width> <int height> <bool repaint> applyUIScaling:<true>
	//		** OR **
	// windows.setWindowPos {<RolloutFloater>|<Rollout>|<int_HWND>|#max} <Box2 rect> <bool repaint> applyUIScaling:<true>
	int count_wo_keys = count_with_keys();
	if (count_wo_keys != 6 && count_wo_keys != 3)
		check_arg_count_with_keys(windows.setWindowPos, 6, count);

	Box2 box;
	BOOL repaint;
	if (count_wo_keys == 6) {
		box.SetXY(arg_list[1]->to_int(), arg_list[2]->to_int());
		box.SetWH(arg_list[3]->to_int(), arg_list[4]->to_int());
		repaint = arg_list[5]->to_bool();
	}
	else {
		box = arg_list[1]->to_box2();
		repaint = arg_list[2]->to_bool();
	}

	Value* tmp;
	BOOL applyUIScaling = bool_key_arg(applyUIScaling, tmp, TRUE); 
	if (applyUIScaling)
	{
		box.left = GetUIScaledValue(box.left);
		box.right = GetUIScaledValue(box.right);
		box.top = GetUIScaledValue(box.top);
		box.bottom = GetUIScaledValue(box.bottom);
	}

	// For a top-level window, the position and dimensions are relative to the upper-left corner of the screen. 
	// For a child window, they are relative to the upper-left corner of the parent window's client area
	HWND hWnd = Value_to_HWND(arg_list[0], _T("windows.setWindowPos"));
	if (repaint)
		InvalidateRect(hWnd, nullptr, TRUE);
	BOOL res = ::MoveWindow(hWnd, box.x(), box.y(), box.w(), box.h(), repaint);
	return bool_result(res);
}

Value* get_window_pos_cf(Value** arg_list, int count)
{
	// <Box2 rect> windows.getWindowPos {<RolloutFloater>|<Rollout>|<int_HWND>|#max} removeUIScaling:<true>
	check_arg_count_with_keys(windows.getWindowRect, 1, count);
	Value* tmp;
	BOOL removeUIScaling = bool_key_arg(removeUIScaling, tmp, TRUE); 

	HWND hWnd = Value_to_HWND(arg_list[0], _T("windows.getWindowPos"));
	RECT wRect;
	// The dimensions are given in screen coordinates that are relative to the upper-left corner of the screen.
	BOOL res = ::GetWindowRect(hWnd, &wRect);

	if (res) {
		MAXScript_TLS* _tls = (MAXScript_TLS*)TlsGetValue(thread_locals_index);
		one_typed_value_local_tls(Box2Value* result);
		// Per Windows GetWindowRect specs: 'In conformance with conventions for the RECT structure, 
		// the bottom-right coordinates of the returned rectangle are exclusive'.
		// So in order to have compatible values in maxscript setWindowPos&getWindowPos,
		// adjusting the results from GetWindowRect to be inclusive
		wRect.right--; wRect.bottom--;
		if (removeUIScaling)
		{
			wRect.left = GetValueUIUnscaled(wRect.left);
			wRect.right = GetValueUIUnscaled(wRect.right);
			wRect.top = GetValueUIUnscaled(wRect.top);
			wRect.bottom = GetValueUIUnscaled(wRect.bottom);
		}
		vl.result = new Box2Value(wRect);
		return_value_tls(vl.result);
	}
	else
		return &undefined;
}

Value* client_to_screen_cf(Value** arg_list, int count)
{
	// <Point2> windows.clientToScreen {<RolloutFloater>|<Rollout>|<int_HWND>|#max} <int x> <int y> applyUIScaling:<true>
	//		** OR **
	// <Point2> windows.clientToScreen {<RolloutFloater>|<Rollout>|<int_HWND>|#max} <Point2 clientCoord> applyUIScaling:<true>
	int count_wo_keys = count_with_keys();
	if (count_wo_keys != 2 && count_wo_keys != 3)
		check_arg_count_with_keys(windows.clientToScreen, 3, count);

	POINT coord;
	if (count_wo_keys == 3) {
		coord.x = arg_list[1]->to_int(); coord.y = arg_list[2]->to_int();
	}
	else {
		Point2 clientCoord = arg_list[1]->to_point2();
		coord.x = clientCoord.x; coord.y = clientCoord.y; 
	}
	Value* tmp;
	BOOL applyUIScaling = bool_key_arg(applyUIScaling, tmp, TRUE); 
	if (applyUIScaling)
	{
		coord.x = GetUIScaledValue(coord.x);
		coord.y = GetUIScaledValue(coord.y);
	}

	HWND hWnd = Value_to_HWND(arg_list[0], _T("windows.clientToScreen"));
	// ClientToScreen returns the new screen coordinates into 'coord' if the function succeeds.
	BOOL res = ::ClientToScreen(hWnd, &coord);
	if (res) {
		if (applyUIScaling)
		{
			coord.x = GetValueUIUnscaled(coord.x);
			coord.y = GetValueUIUnscaled(coord.y);
		}
		MAXScript_TLS* _tls = (MAXScript_TLS*)TlsGetValue(thread_locals_index);
		one_typed_value_local_tls(Point2Value* result);
		vl.result = new Point2Value(coord);
		return_value_tls(vl.result);
	}
	else
		return &undefined;
}

Value* screen_to_client_cf(Value** arg_list, int count)
{
	// <Point2> windows.screenToClient {<RolloutFloater>|<Rollout>|<int_HWND>|#max} <int x> <int y> applyUIScaling:<true>
	//		** OR **
	// <Point2> windows.screenToClient {<RolloutFloater>|<Rollout>|<int_HWND>|#max} <Point2 screenCoord> applyUIScaling:<true>
	int count_wo_keys = count_with_keys();
	if (count_wo_keys != 2 && count_wo_keys != 3)
		check_arg_count_with_keys(windows.screenToClient, 3, count);

	POINT coord;
	if (count_wo_keys == 3) {
		coord.x = arg_list[1]->to_int(); coord.y = arg_list[2]->to_int();
	}
	else {
		Point2 screenCoord = arg_list[1]->to_point2();
		coord.x = screenCoord.x; coord.y = screenCoord.y; 
	}
	Value* tmp;
	BOOL applyUIScaling = bool_key_arg(applyUIScaling, tmp, TRUE); 
	if (applyUIScaling)
	{
		coord.x = GetUIScaledValue(coord.x);
		coord.y = GetUIScaledValue(coord.y);
	}

	HWND hWnd = Value_to_HWND(arg_list[0], _T("windows.screenToClient"));
	// ScreenToClient returns the new client coordinates into 'coord' if the function succeeds.
	BOOL res = ::ScreenToClient(hWnd, &coord);
	if (res) {
		if (applyUIScaling)
		{
			coord.x = GetValueUIUnscaled(coord.x);
			coord.y = GetValueUIUnscaled(coord.y);
		}
		MAXScript_TLS* _tls = (MAXScript_TLS*)TlsGetValue(thread_locals_index);
		one_typed_value_local_tls(Point2Value* result);
		vl.result = new Point2Value(coord);
		return_value_tls(vl.result);
	}
	else
		return &undefined;
}

Value* snapshot_cf(Value** arg_list, int count)
{
	// windows.snapshot {<RolloutFloater>|<Rollout>|<int_HWND>|#max|undefined}  captureAlpha:<bool> gammaCorrect:<bool> captureScreenPixels:<bool>
	check_arg_count_with_keys(windows.snapshot, 1, count);
	bool captureAlpha = key_arg_or_default(captureAlpha, &false_value)->to_bool() ? true : false;
	bool gammaCorrect = key_arg_or_default(gammaCorrect, &true_value)->to_bool() ? true : false;
	bool captureScreenPixels  = key_arg_or_default(captureScreenPixels, &false_value)->to_bool() ? true : false;
	HWND hWndDesktop = GetDesktopWindow();
	HWND hWnd = Value_to_HWND(arg_list[0], _T("windows.snapshot"));
	if (hWnd == nullptr)
		hWnd = hWndDesktop;

	if (hWnd == hWndDesktop)
		captureScreenPixels = false;  // capturing desktop, so meaningless

	if (IsWindow(hWnd)) 
	{
		int width = 0;
		int height = 0;
		int capture_offset_x = 0;
		int capture_offset_y = 0;
		if (hWnd == hWndDesktop)
		{
			width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
			height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
		}
		else
		{
			WINDOWPLACEMENT wp;
			wp.length = sizeof(WINDOWPLACEMENT);
			GetWindowPlacement(hWnd, &wp);

			if (wp.showCmd != SW_SHOWMINIMIZED)
			{
				WINDOWINFO winfo;
				winfo.cbSize = sizeof(WINDOWINFO);
				GetWindowInfo(hWnd, &winfo);

				RECT rect;
				if (captureScreenPixels)
				{
					if (wp.showCmd == SW_SHOWMAXIMIZED) // no border
					{
						winfo.rcWindow.top += winfo.cyWindowBorders;
						winfo.rcWindow.bottom -= winfo.cyWindowBorders;
						winfo.rcWindow.left += winfo.cxWindowBorders;
						winfo.rcWindow.right -= winfo.cxWindowBorders;
						rect = winfo.rcWindow;
					}
					else  // border
					{
						GetWindowRect(hWnd, &rect);
					}
					capture_offset_x = rect.left;
					capture_offset_y = rect.top;
				}
				else // no border
				{
					winfo.rcWindow.top += winfo.cyWindowBorders;
					winfo.rcWindow.bottom -= winfo.cyWindowBorders;
					winfo.rcWindow.left += winfo.cxWindowBorders;
					winfo.rcWindow.right -= winfo.cxWindowBorders;
					rect = winfo.rcWindow;
					capture_offset_x = winfo.rcWindow.left - winfo.rcClient.left;
					capture_offset_y = winfo.rcWindow.top - winfo.rcClient.top;
				}
				width = (rect.right-rect.left);
				height = (rect.bottom-rect.top);
			}
		}
		int bufferSize =  width * height * sizeof(RGBQUAD);
		int dibSize = sizeof(BITMAPINFOHEADER) + bufferSize;

		BITMAPINFO *bmi = (BITMAPINFO *)malloc(dibSize);
		if (bmi == nullptr)
			return &undefined;
		memset(bmi, 0, dibSize);
		BYTE *buf = new BYTE[dibSize];
		if (buf == nullptr)
		{
			free(bmi);
			return &undefined;
		}

		HWND hWndToCaptureFrom = hWnd;
		int capture_width = width;
		int capture_height = height;
		if (captureScreenPixels)
		{
			hWndToCaptureFrom = hWndDesktop;
			capture_width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
			capture_height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
		}
		HDC hdc = GetDC(hWndToCaptureFrom);
		HBITMAP hBitmap = CreateCompatibleBitmap(hdc, capture_width, capture_height);
		HDC hdcMem = CreateCompatibleDC(hdc);
		HGDIOBJ hOldB = SelectObject(hdcMem, hBitmap);
		BitBlt(hdcMem, 0, 0, width, height, hdc, capture_offset_x, capture_offset_y, SRCCOPY);

		BITMAPINFOHEADER *bmih = &(bmi->bmiHeader);
		bmih->biSize = sizeof(BITMAPINFOHEADER);
		bmih->biWidth = width;
		bmih->biHeight = height;
		bmih->biPlanes = 1;
		bmih->biBitCount = 32;
		bmih->biCompression = BI_RGB;
		bmih->biSizeImage = bufferSize;
		bmih->biXPelsPerMeter = GetDeviceCaps(hdc, LOGPIXELSX);
		bmih->biYPelsPerMeter = GetDeviceCaps(hdc, LOGPIXELSY);
		bmih->biClrUsed = 0;
		bmih->biClrImportant = 0;

		if (!GetDIBits(hdcMem, hBitmap, 0, height, &buf[sizeof(BITMAPINFOHEADER)], bmi, DIB_RGB_COLORS))
		{
			delete [] buf;
			free(bmi);
			DeleteDC(hdcMem);
			ReleaseDC(hWnd, hdc);
			DeleteObject(hBitmap);
			return &undefined;
		}			
		memcpy(buf, bmih, sizeof(BITMAPINFOHEADER));

		Bitmap *map = CreateBitmapFromBInfo((void**)&buf, width, height, captureAlpha);

		delete [] buf;
		free(bmi);
		DeleteDC(hdcMem);
		ReleaseDC(hWnd, hdc);
		DeleteObject(hBitmap);

		if (gammaCorrect) 
			DeGammaCorrectBitmap(map);

		MAXScript_TLS* _tls = (MAXScript_TLS*)TlsGetValue(thread_locals_index);
		one_typed_value_local_tls(MAXBitMap* mbm);
		vl.mbm = new MAXBitMap ();
		vl.mbm->bi.CopyImageInfo(&map->Storage()->bi);
		vl.mbm->bi.SetFirstFrame(0);
		vl.mbm->bi.SetLastFrame(0);
		vl.mbm->bi.SetName(_T(""));
		vl.mbm->bi.SetDevice(_T(""));
		if (vl.mbm->bi.Type() > BMM_TRUE_64)
			vl.mbm->bi.SetType(BMM_TRUE_64);
		vl.mbm->bm = map;
		return_value_tls(vl.mbm);
	}
	else
		return &undefined;
}

Value* getCapture_cf(Value** arg_list, int count)
{
	// windows.getCapture()
	check_arg_count(windows.getCapture, 0, count);
	return_value(IntegerPtr::intern((INT_PTR)GetCapture()));
}

Value* getFocus_cf(Value** arg_list, int count)
{
	// windows.getFocus()
	check_arg_count(windows.getFocus, 0, count);
	return_value(IntegerPtr::intern((INT_PTR)GetFocus()));
}

Value* isWindowEnabled_cf(Value** arg_list, int count)
{
	// windows.isWindowEnabled  {<RolloutFloater>|<Rollout>|<int_HWND>|#max}
	check_arg_count(windows.isWindowEnabled, 1, count);
	HWND hWnd = Value_to_HWND(arg_list[0], _T("windows.isWindowEnabled"));
	return (IsWindow(hWnd) && IsWindowEnabled(hWnd)) ? &true_value : &false_value;
}

Value* isWindow_cf(Value** arg_list, int count)
{
	// windows.isWindow  {<RolloutFloater>|<Rollout>|<int_HWND>|#max}
	check_arg_count(windows.isWindow, 1, count);
	HWND hWnd = Value_to_HWND(arg_list[0], _T("windows.isWindow"));
	return (IsWindow(hWnd)) ? &true_value : &false_value;
}

Value* isWindowVisible_cf(Value** arg_list, int count)
{
	// windows.isWindowVisible  {<RolloutFloater>|<Rollout>|<int_HWND>|#max}
	check_arg_count(windows.isWindowVisible, 1, count);
	HWND hWnd = Value_to_HWND(arg_list[0], _T("windows.isWindowVisible"));
	return (IsWindow(hWnd) && IsWindowVisible(hWnd)) ? &true_value : &false_value;
}

Value* showWindow_cf(Value** arg_list, int count)
{
	// windows.showWindow {<RolloutFloater>|<Rollout>|<int_HWND>|#max} <enum showCmd>
	check_arg_count(windows.showWindow, 2, count);
	HWND hWnd = Value_to_HWND(arg_list[0], _T("windows.showWindow"));
	def_showwindow_commands();
	int which = ConvertValueToID(showwindow_commands, _countof(showwindow_commands), arg_list[1]);
	return (IsWindow(hWnd) && ShowWindow(hWnd, which)) ? &true_value : &false_value;
}

Value* getWindowPlacement_cf(Value** arg_list, int count)
{
	// windows.getWindowPlacement {<RolloutFloater>|<Rollout>|<int_HWND>|#max} removeUIScaling:<true>
	check_arg_count_with_keys(windows.getWindowPlacement, 1, count);
	HWND hWnd = Value_to_HWND(arg_list[0], _T("windows.getWindowPlacement"));
	Value* tmp;
	BOOL removeUIScaling = bool_key_arg(removeUIScaling, tmp, TRUE);
	WINDOWPLACEMENT windowplacement; 
	windowplacement.length = sizeof(windowplacement);
	if (IsWindow(hWnd) && GetWindowPlacement(hWnd, &windowplacement))
	{
		MAXScript_TLS* _tls = (MAXScript_TLS*)TlsGetValue(thread_locals_index);
		one_typed_value_local_tls(Array * result);
		vl.result = new Array(5);
		def_showwindow_commands();
		Value* which = ConvertIDToValue(showwindow_commands, _countof(showwindow_commands), windowplacement.showCmd, &undefined);
		vl.result->append(which);
		if (removeUIScaling)
		{
			if (windowplacement.ptMinPosition.x != -1)
				windowplacement.ptMinPosition.x = GetValueUIUnscaled(windowplacement.ptMinPosition.x);
			if (windowplacement.ptMinPosition.y != -1)
				windowplacement.ptMinPosition.y = GetValueUIUnscaled(windowplacement.ptMinPosition.y);
			if (windowplacement.ptMaxPosition.x != -1)
				windowplacement.ptMaxPosition.x = GetValueUIUnscaled(windowplacement.ptMaxPosition.x);
			if (windowplacement.ptMaxPosition.y != -1)
				windowplacement.ptMaxPosition.y = GetValueUIUnscaled(windowplacement.ptMaxPosition.y);
			windowplacement.rcNormalPosition.bottom = GetValueUIUnscaled(windowplacement.rcNormalPosition.bottom);
			windowplacement.rcNormalPosition.left = GetValueUIUnscaled(windowplacement.rcNormalPosition.left);
			windowplacement.rcNormalPosition.right = GetValueUIUnscaled(windowplacement.rcNormalPosition.right);
			windowplacement.rcNormalPosition.top = GetValueUIUnscaled(windowplacement.rcNormalPosition.top);
		}
		vl.result->append(new Point2Value(windowplacement.ptMinPosition));
		vl.result->append(new Point2Value(windowplacement.ptMaxPosition));
		vl.result->append(new Box2Value(windowplacement.rcNormalPosition));
		vl.result->append(Integer::intern(windowplacement.flags));
		return_value_tls(vl.result);
	}
	return &undefined;
}

Value* setWindowPlacement_cf(Value** arg_list, int count)
{
	// windows.setWindowPlacement {<RolloutFloater>|<Rollout>|<int_HWND>|#max} <enum showCmd> <point2 minPos> <point2 maxPos> <box2 normalPos> <flags> applyUIScaling:<true>
	// windows.setWindowPlacement {<RolloutFloater>|<Rollout>|<int_HWND>|#max} <array> applyUIScaling:<true>
	int posArgCount = count_with_keys();
	Value* showCmd;
	Value* minPos;
	Value* maxPos;
	Value* normalPos;
	Value* flags;
	if (posArgCount != 2)
	{
		check_arg_count_with_keys(windows.setWindowPlacement, 6, count);
		showCmd = arg_list[1];
		minPos = arg_list[2];
		maxPos = arg_list[3];
		normalPos = arg_list[4];
		flags = arg_list[5];
	}
	else
	{
		type_check(arg_list[1], Array, _T("windows.setWindowPlacement"));
		Array* args = (Array*)arg_list[1];
		if (args->size != 5)
			throw RuntimeError(MaxSDK::GetResourceStringAsMSTR(IDS_SETWINDOWPLACEMENT_NEEDS_5_ARRAY), args);
		showCmd = args->data[0];
		minPos = args->data[1];
		maxPos = args->data[2];
		normalPos = args->data[3];
		flags = args->data[4];
	}
	HWND hWnd = Value_to_HWND(arg_list[0], _T("windows.setWindowPlacement"));
	Value* tmp;
	BOOL applyUIScaling = bool_key_arg(applyUIScaling, tmp, TRUE);
	if (IsWindow(hWnd))
	{
		WINDOWPLACEMENT windowplacement;
		windowplacement.length = sizeof(windowplacement);
		windowplacement.flags = flags->to_int();
		def_showwindow_commands();
		windowplacement.showCmd = ConvertValueToID(showwindow_commands, _countof(showwindow_commands), showCmd);
		Point2 p = minPos->to_point2();
		windowplacement.ptMinPosition = { (long)p.x, (long)p.y };
		p = maxPos->to_point2();
		windowplacement.ptMaxPosition = { (long)p.x, (long)p.y };
		windowplacement.rcNormalPosition = normalPos->to_box2();
		if (applyUIScaling)
		{

			if (windowplacement.ptMinPosition.x != -1)
				windowplacement.ptMinPosition.x = GetUIScaledValue(windowplacement.ptMinPosition.x);
			if (windowplacement.ptMinPosition.y != -1)
				windowplacement.ptMinPosition.y = GetUIScaledValue(windowplacement.ptMinPosition.y);
			if (windowplacement.ptMaxPosition.x != -1)
				windowplacement.ptMaxPosition.x = GetUIScaledValue(windowplacement.ptMaxPosition.x);
			if (windowplacement.ptMaxPosition.y != -1)
				windowplacement.ptMaxPosition.y = GetUIScaledValue(windowplacement.ptMaxPosition.y);
			windowplacement.rcNormalPosition.bottom = GetUIScaledValue(windowplacement.rcNormalPosition.bottom);
			windowplacement.rcNormalPosition.left = GetUIScaledValue(windowplacement.rcNormalPosition.left);
			windowplacement.rcNormalPosition.right = GetUIScaledValue(windowplacement.rcNormalPosition.right);
			windowplacement.rcNormalPosition.top = GetUIScaledValue(windowplacement.rcNormalPosition.top);
		}
		if (SetWindowPlacement(hWnd, &windowplacement))
			return &true_value;
	}
	return &false_value;
}

Value* setFocus_cf(Value** arg_list, int count)
{
	// setFocus {<RolloutControl>|<RolloutFloater>|<Rollout>|<int_HWND>|#max} <bool>
	check_arg_count(setFocus, 1, count);
	if (is_rolloutcontrol(arg_list[0]))
	{
		RolloutControl* roc = (RolloutControl*)arg_list[0];
		return roc->set_focus() ? &true_value : &false_value;
	}
	else
	{
		HWND hWnd = Value_to_HWND(arg_list[0], _T("setFocus"), true);
		return (IsWindow(hWnd) && SetFocus(hWnd)) ? &true_value : &false_value;
	}
}

Value* windows_setFocus_cf(Value** arg_list, int count)
{
	// windows.setFocus {<RolloutControl>|<RolloutFloater>|<Rollout>|<int_HWND>|#max} <bool>
	check_arg_count(windows.setFocus, 1, count);
	if (!is_rolloutcontrol(arg_list[0]))
		Value_to_HWND(arg_list[0], _T("windows.setFocus"), true);
	return setFocus_cf(arg_list, count);
}

Value* setForegroundWindow_cf(Value * *arg_list, int count)
{
	// setForegroundWindow {<RolloutFloater>|<Rollout>|<int_HWND>|#max}
	check_arg_count(setForegroundWindow, 1, count);
	HWND hWnd = Value_to_HWND(arg_list[0], _T("setForegroundWindow"));
	return (IsWindow(hWnd) && SetForegroundWindow(hWnd)) ? &true_value : &false_value;
}

Value* windows_setForegroundWindow_cf(Value** arg_list, int count)
{
	// windows.setForegroundWindow {<RolloutFloater>|<Rollout>|<int_HWND>|#max}
	check_arg_count(windows.setForegroundWindow, 1, count);
	HWND hWnd = Value_to_HWND(arg_list[0], _T("windows.setForegroundWindow"));
	return setForegroundWindow_cf(arg_list, count);
}

Value* windows_getWindowLong_cf(Value** arg_list, int count)
{
	// windows.getWindowLong {<RolloutFloater>|<Rollout>|<int_HWND>|#max} {#exStyle|#instance|#id|#style|#userData|#wndproc|#dlgproc|#msgResult|#user|<int>}
	check_arg_count(windows.getWindowLong, 2, count);
	HWND hWnd = Value_to_HWND(arg_list[0], _T("windows.getWindowLong"));
	if (IsWindow(hWnd))
	{
		int index;
		if (is_name(arg_list[1]))
		{
			def_windowlong_index();
			index = ConvertValueToID(windowlong_index, _countof(windowlong_index), arg_list[1]);
		}
		else
			index = arg_list[1]->to_int();
		SetLastError(0);
		size_t val = GetWindowLongPtr(hWnd, index);
		if (val == 0)
		{
			DWORD errorCode = GetLastError();
			if (errorCode != 0)
				return_value(&undefined);
		}
		return_value(IntegerPtr::intern(val));
	}
	return &undefined;
}

Value* windows_setWindowLong_cf(Value** arg_list, int count)
{
	// windows.setWindowLong {<RolloutFloater>|<Rollout>|<int_HWND>|#max} {#exStyle|#instance|#id|#style|#userData|#wndproc|#dlgproc|#msgResult|#user|<int>} <intptr>
	check_arg_count(windows.setWindowLong, 3, count);
	HWND hWnd = Value_to_HWND(arg_list[0], _T("windows.setWindowLong"));
	if (IsWindow(hWnd))
	{
		int index;
		if (is_name(arg_list[1]))
		{
			def_windowlong_index();
			index = ConvertValueToID(windowlong_index, _countof(windowlong_index), arg_list[1]);
		}
		else
			index = arg_list[1]->to_int();
		size_t val = arg_list[2]->to_intptr();
		SetLastError(0);
		size_t oldVal = SetWindowLongPtr(hWnd, index, val);
		if (oldVal == 0)
		{
			DWORD errorCode = GetLastError();
			if (errorCode != 0)
				return_value(&undefined);
		}
		return_value(IntegerPtr::intern(oldVal));
	}
	return &undefined;
}

Value* windowFromPoint_cf(Value** arg_list, int count)
{
	// <IntegerPtr> windows.windowFromPoint <point2> applyUIScaling:<bool>
	check_arg_count(windows.windowFromPoint, 1, count);
	Point2 coord = arg_list[0]->to_point2();
	POINT pt{ (int)coord.x, (int)coord.y };

	Value* tmp;
	BOOL applyUIScaling = bool_key_arg(applyUIScaling, tmp, TRUE);
	if (applyUIScaling)
	{
		pt.x = MAXScript::GetUIScaledValue(pt.x);
		pt.y = MAXScript::GetUIScaledValue(pt.y);
	}

	return_value(IntegerPtr::intern((INT_PTR)WindowFromPoint(pt)));
}