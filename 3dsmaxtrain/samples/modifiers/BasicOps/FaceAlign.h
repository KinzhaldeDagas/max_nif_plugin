/**********************************************************************
 *<
	FILE: FaceAlign.h

	DESCRIPTION: Classes to support face picking for gizmo alignment

	CREATED BY: Mathew Kaustinen

	HISTORY: created 10/27/2020

 *>	Copyright (c) 2020, All Rights Reserved.
 **********************************************************************/

#include "BasicOps.h"

#define CID_FACEALIGNMOD	(CID_USER+0x8086)

// Class to allow alignment operations to querry the object requesting alignment
// and to update it's UI
class ObjectAlignInterface
{
public:
	virtual bool	IsRemoveNegative() { return true; };
	virtual Matrix3	GetControllerTM(TimeValue t) { return Matrix3{}; };
	virtual Matrix3 GetRefObjectTM(TimeValue t) { return Matrix3{}; };
	virtual void	SetControllerPacket(TimeValue t, SetXFormPacket &sxfp) {};
	virtual void	Move(TimeValue t, Point3 &p) {};
	virtual bool	HasObject() { return false; };

	virtual void	EnterPick() {};
	virtual void	ExitPick() {};

	virtual void	PreRotate(Matrix3& m) { m.PreRotateZ(PI); };
};

class FaceAlignMouseProc : public MouseCallBack {
public:
	ObjectAlignInterface *oai;
	IObjParam *ip;
	int	cmdID;

	FaceAlignMouseProc(ObjectAlignInterface *o, IObjParam *i) {
		oai = o;ip = i;
		cmdID = i->GetCommandMode()->ID();
	}

	int proc(HWND hWnd, int msg, int point, int flags, IPoint2 m);
	BOOL FaceAlignMap(HWND hWnd, IPoint2 m, BOOL bCapture = FALSE);
	void MatrixFromNormal(Point3& normal, Matrix3& mat);

	void StoreMode()
	{
		cmdID = ip->GetCommandMode()->ID();
	}

	void RestoreMode()
	{
		if (cmdID < CID_OBJSELECT)
		{
			ip->SetStdCommandMode(cmdID);
		}
		else
			ip->SetStdCommandMode(CID_OBJSELECT);
	}

};

class FaceAlignMode : public CommandMode {
public:
	ChangeFGObject fgProc;
	FaceAlignMouseProc alignproc;
	IObjParam *ip;
	ObjectAlignInterface *oai;

	BOOL	bActive;

	FaceAlignMode(Modifier *m, ObjectAlignInterface* o, IObjParam *i)
		: fgProc((ReferenceTarget*)m),
		alignproc(o, i)
	{
		ip = i; oai = o;
		bActive = FALSE;
	}

	~FaceAlignMode()
	{
		if (bActive)
			alignproc.RestoreMode();
	}

	int Class() { return PICK_COMMAND; }
	int ID() { return CID_FACEALIGNMOD; }
	MouseCallBack *MouseProc(int *numPoints) { *numPoints = 2;return &alignproc; }
	ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
	BOOL ChangeFG(CommandMode *oldMode) { return oldMode->ChangeFGProc() != &fgProc; }
	void EnterMode();
	void ExitMode();
	void AlignPressed();
};