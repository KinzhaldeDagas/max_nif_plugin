//////////////////////////////////////////////////////////////////////////////
//
//  Copyright 2009 Autodesk, Inc.  All rights reserved.
//  Copyright 2003 Character Animation Technologies.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//
//////////////////////////////////////////////////////////////////////////////

#include "../CATControls/cat.h"
#include "resource.h"
#include "ProgressWindow.h"

//
// This function registers a window class for the progress
// window.  The callback ProgressWindowProc() calls a proc in
// the static instance of our progress window class.
//
LRESULT CALLBACK ProgressWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#define PROGRESSWINDOWCLASS _T("CATProgressWindow")

BOOL RegisterProgressWindowClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcx;

	wcx.cbSize = sizeof(wcx);
	wcx.style = CS_HREDRAW | CS_VREDRAW;
	wcx.lpfnWndProc = ProgressWindowProc;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = hInstance;
	wcx.hIcon = NULL;//LoadIcon(NULL, IDI_APPLICATION);
	wcx.hCursor = LoadCursor(NULL, IDC_WAIT);
	wcx.hbrBackground = NULL;
	wcx.lpszMenuName = NULL;
	wcx.lpszClassName = PROGRESSWINDOWCLASS;
	wcx.hIconSm = NULL;

	return RegisterClassEx(&wcx);
}

//////////////////////////////////////////////////////////////////////
// class ProgressWindow
//
// This wraps a Win32 window which displays progress.  Users can
// call methods on this class using the IProgressWindow interface,
// which is obtained by calling the function GetProgressWindow().
//
class ProgressWindow : public IProgressWindow
{
	HWND hWnd;

	int nPercent;
	TCHAR *szCaption;
	TCHAR *szMessage;

	MaskedBitmap *mbmPawIcon;

	void RedrawWindow(BOOL bErase = TRUE) { if (hWnd) InvalidateRect(hWnd, NULL, bErase); }

	void EraseBackground(HDC hdc);
	void Paint(HDC hdc, BOOL bErase);
	void PaintMessage(HDC hdc);
	void PaintProgress(HDC hdc);

public:
	ProgressWindow();
	~ProgressWindow();
	LRESULT CALLBACK Proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	//
	// IProgressWindow implementation.
	//
	BOOL ProgressBegin(const TCHAR *szCaption, const TCHAR *szMessage);
	void ProgressEnd();

	void SetCaption(const TCHAR *szCaption);
	void SetMessage(const TCHAR *szMessage);
	void SetPercent(int nPercent);
};

//
// This is the single static global instance of the ProgressWindow
// class.  A user may get a pointer to it by calling the function
// GetProgressWindow().
//
static ProgressWindow globalProgressWindow;

IProgressWindow *GetProgressWindow() {
	return &globalProgressWindow;
}

//
// This is registered as the window procedure for the ProgressWindow
// wndclass.  It simply passes all messages through to a member funtion
// in the global instance of the ProgressWindow class.
//
LRESULT CALLBACK ProgressWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return globalProgressWindow.Proc(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK ProgressWindow::Proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CREATE:
		this->hWnd = hWnd;
		{
			//
			// Load the CATPawIcon masked bitmap.  This contains a mask that
			// anti-aliases the icon with the background.
			//
			// TODO: make this a bitmap
		/*	HDC hdc = GetDC(hWnd);
			HBITMAP hbmPawImage = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_CAT2LOGO_SMALL));
			HBITMAP hbmPawMask = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_CAT2LOGO_SMALL));
			mbmPawIcon = new MaskedBitmap(hdc, 32, 32, hbmPawImage, hbmPawMask);
			ReleaseDC(hWnd, hdc);
		*/
		//
		// Set the window size
		//
			RECT rcClient, rcDesiredClient, rcWindow;
			GetClientRect(hWnd, &rcClient);
			GetWindowRect(hWnd, &rcWindow);
			SetRect(&rcDesiredClient, 0, 0, 300, 150);

			//int nWidth = rcWindow.right + rcDesiredClient.right - rcClient.right;
			//int nHeight = rcWindow.bottom + rcDesiredClient.bottom - rcClient.bottom;
//				SetWindowPos(hWnd, NULL, 0, 0, nWidth, nHeight, SWP_NOZORDER | SWP_NOMOVE);

			CenterWindow(hWnd, GetParent(hWnd));
		}
		return 1;

	case WM_NCDESTROY:
		this->hWnd = NULL;
		break;

		// Window display functions.  We must implement WM_ERASEBKGND because
		// we have set the hbrBackground field of the WNDCLASSEX to NULL.  All
		// painting is handled by class member functions so we can force a
		// repaint without having to send WM_PAINT.  If the window is enabled
		// or disabled, we invalidate (this might happen already).
	case WM_ERASEBKGND:
	{
		//HDC hdc = (HDC)wParam;
		//EraseBackground(hdc);
	}
	return 1;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		if (hdc) {
			Paint(hdc, TRUE);
			EndPaint(hWnd, &ps);
		}
	}
	return 1;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

//
// Erases the window background.
//
void ProgressWindow::EraseBackground(HDC hdc)
{
	if (!hdc) return;
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);
	FillRect(hdc, &rcClient, ColorMan()->GetBrush(kBackground));
}

//
// Draws the contents of the window, optionally erasing the content
// beforehand.  To update the progress bar only, call PaintProgress().
// This function is usually only called as a result of a WM_PAINT.
//
void ProgressWindow::Paint(HDC hdc, BOOL bErase)
{
	if (!hdc) return;

	// Erase the client area with the background window colour.
	if (bErase) EraseBackground(hdc);

	// Display the catpaw icon
//	mbmPawIcon->MaskBlit(hdc, 4, 10);
	PaintMessage(hdc);
	PaintProgress(hdc);
}

//
// Paints the progress bar in a sunken border.
//
void ProgressWindow::PaintMessage(HDC hdc)
{
	if (!hdc) return;

	RECT rcMessage;
	SetRect(&rcMessage, 40, 10, 296, 26);

	SetTextColor(hdc, ColorMan()->GetColor(kText));
	SetBkColor(hdc, ColorMan()->GetColor(kBackground));
	EraseBackground(hdc);
	DrawText(hdc, szMessage, (int)_tcslen(szMessage), &rcMessage, DT_TOP | DT_SINGLELINE | DT_WORD_ELLIPSIS);
}

//
// Paints the progress bar in a sunken border.
//
void ProgressWindow::PaintProgress(HDC hdc)
{
	if (!hdc) return;

	RECT rcProgress;
	SetRect(&rcProgress, 40, 32, 296, 42);

	// Draw the sunken client area
	HPEN hDarkPen = CreatePen(PS_SOLID, 0, ColorMan()->GetColor(kShadow));
	HPEN hLightPen = CreatePen(PS_SOLID, 0, ColorMan()->GetColor(kHilight));
	HGDIOBJ hOldPen = SelectObject(hdc, hDarkPen);

	MoveToEx(hdc, rcProgress.left, rcProgress.bottom, NULL);
	LineTo(hdc, rcProgress.left, rcProgress.top);
	LineTo(hdc, rcProgress.right, rcProgress.top);
	SelectObject(hdc, hLightPen);
	LineTo(hdc, rcProgress.right, rcProgress.bottom);
	LineTo(hdc, rcProgress.left, rcProgress.bottom);

	SelectObject(hdc, hOldPen);
	DeleteObject(hDarkPen);
	DeleteObject(hLightPen);

	// Draw the progress bar and background.
	int nPercentPos = rcProgress.left + (rcProgress.right - rcProgress.left) * nPercent / 100;

	rcProgress.left += 1;
	rcProgress.top += 1;
	RECT rcPercent = rcProgress;

	rcPercent.right = nPercentPos + 1;
	FillRect(hdc, &rcPercent, GetSysColorBrush(COLOR_HIGHLIGHT));
	rcPercent.left = rcPercent.right;
	rcPercent.right = rcProgress.right;
	FillRect(hdc, &rcPercent, ColorMan()->GetBrush(kWindow));
}

ProgressWindow::ProgressWindow() : nPercent(0)
{
	hWnd = NULL;
	szCaption = NULL;
	szMessage = NULL;
	mbmPawIcon = NULL;
}

ProgressWindow::~ProgressWindow()
{
	//	delete mbmPawIcon;
	free(szCaption);
	free(szMessage);
}

//
// Opens the progress window if it is not already open, and
// sets the caption and message.  Returns TRUE if the window
// was opened successfully, or FALSE if it is already open
// or some other error occurred.  If the window is already
// open, we do NOT set the caption or message.  To do that,
// call SetCaption() and Setmessage().
//
BOOL ProgressWindow::ProgressBegin(const TCHAR *szCaption, const TCHAR *szMessage)
{
	if (hWnd) {
		// The progress bar is already open.
		// Simply update it with the new labels
		SetPercent(0);
		SetCaption(szCaption);
		SetMessage(szMessage);
		return TRUE;
	}

	HWND hProgressWnd;

	SetPercent(0);
	SetCaption(szCaption);
	SetMessage(szMessage);

	RegisterProgressWindowClass(hInstance);

	hProgressWnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		PROGRESSWINDOWCLASS,
		NULL, //this->szCaption,
		WS_POPUP | WS_BORDER | WS_VISIBLE,
		0, 0, 310, 58,
		GetCOREInterface()->GetMAXHWnd(),
		NULL,
		hInstance,
		NULL);

	BOOL bSuccess = (hProgressWnd ? TRUE : FALSE);

	// I just can't get the window to receive WM_PAINT messages when
	// it's invoked from a modal window.  Actually, the window receives
	// bugger all messages.  So here, force an immediate paint.  All
	// updates will bypass WM_PAINT messages as usual.  In total the
	// window is likely to receive absolutely no WM_PAINTS during its
	// entire life.
	if (bSuccess && hWnd) {
		HDC hdc = GetDC(hWnd);
		Paint(hdc, TRUE);
		ReleaseDC(hWnd, hdc);
	}

	return bSuccess;
}

//
// Closes the progress window
//
void ProgressWindow::ProgressEnd()
{
	if (hWnd) {
		DestroyWindow(hWnd);
	}
}

//
// Changes the window caption.
//
void ProgressWindow::SetCaption(const TCHAR *szCaption)
{
	if (this->szCaption) free(this->szCaption);
	this->szCaption = _tcsdup(szCaption ? szCaption : _T(""));
	if (hWnd) SetWindowText(hWnd, this->szCaption);
}

//
// Changes the progress message and redraws the message only.
//
void ProgressWindow::SetMessage(const TCHAR *szMessage)
{
	if (this->szMessage) free(this->szMessage);
	this->szMessage = _tcsdup(szMessage ? szMessage : _T(""));
	if (hWnd) {
		HDC hdc = GetDC(hWnd);
		PaintMessage(hdc);
		ReleaseDC(hWnd, hdc);
	}
}

//
// Sets the new percentage value and redraws the progress
// bar only.
//
void ProgressWindow::SetPercent(int nPercent)
{
	// Clamp the percent to a valid range.
	this->nPercent = std::clamp(nPercent, 0, 100);
	if (hWnd) {
		HDC hdc = GetDC(hWnd);
		PaintProgress(hdc);
		ReleaseDC(hWnd, hdc);
		//		UpdateWindow(hWnd);

		/*		MSG msg;
				while (::PeekMessage(&msg, hWnd, 0, 0, PM_NOREMOVE)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
		*/
	}
}
