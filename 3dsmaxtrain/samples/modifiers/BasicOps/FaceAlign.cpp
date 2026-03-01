/**********************************************************************
 *<
	FILE: FaceAlign.cpp

	DESCRIPTION: Classes to support face picking for gizmo alignment

	CREATED BY: Mathew Kaustinen

	HISTORY: created 10/27/2020

 *>	Copyright (c) 2020, All Rights Reserved.
 **********************************************************************/

#include "FaceAlign.h"
#include "decomp.h"

void FaceAlignMouseProc::MatrixFromNormal(Point3& normal, Matrix3& mat)
{
	Point3 vx;
	vx.z = .0f;
	vx.x = -normal.y;
	vx.y = normal.x;
	if (vx.x == .0f && vx.y == .0f) {
		vx.x = 1.0f;
	}
	mat.SetRow(0, vx);
	mat.SetRow(1, normal^vx);
	mat.SetRow(2, normal);
	mat.SetTrans(Point3(0, 0, 0));
	mat.NoScale();
}

BOOL FaceAlignMouseProc::FaceAlignMap(HWND hWnd, IPoint2 m, BOOL bCapture)
{
	BOOL		bFound = FALSE;

	ViewExp &vpt = ip->GetViewExp(hWnd);
	if (!oai || !vpt.IsAlive())
		return bFound;

	Ray ray, wray;
	float at;
	TimeValue t = ip->GetTime();
	GeomObject *obj;
	Point3 norm, pt;
	Interval valid;

	// Get mod contexts and nodes for this modifier
	ModContextList mcList;
	INodeTab nodeList;
	ip->GetModContexts(mcList, nodeList);

	// Calculate a ray from the mouse point
	vpt.MapScreenToWorldRay(float(m.x), float(m.y), wray);

	for (int i = 0; i < nodeList.Count(); i++) {
		INode *node = nodeList[i];

		// Get the object from the node
		ObjectState os = node->EvalWorldState(t);
		if (os.obj->SuperClassID() == GEOMOBJECT_CLASS_ID) {
			obj = (GeomObject*)os.obj;
		}
		else {
			continue;
		}

		// Back transform the ray into object space.		
		Matrix3 obtm = node->GetObjectTM(t);
		Matrix3 iobtm = Inverse(obtm);
		ray.p = iobtm * wray.p;
		ray.dir = VectorTransform(iobtm, wray.dir);

		// See if we hit the object
		if (obj->IntersectRay(t, ray, at, norm))
		{
			bFound = TRUE;

			if (!bCapture)
				break;

			// Calculate the hit point 
			pt = ray.p + ray.dir * at;

			// Get the mod context tm
			if (mcList[0]->tm)
			{
				Matrix3 tm = *mcList[0]->tm;

				// Transform the point and ray into mod context space
				norm = Normalize(VectorTransform(tm, norm));
				pt = pt * tm;
			}

			// move the hit point slightly above / below the surface
			constexpr float	eps = 1e-4f;
			if (oai->IsRemoveNegative())
				pt += norm * eps;
			else
				pt -= norm * eps;

			// Construct the target transformation in mod context space
			// Since we are in Mod Context space, need not worry about scaling
			Matrix3 destTM;
			MatrixFromNormal(norm, destTM);
			destTM.SetTrans(pt);
			oai->PreRotate(destTM);

			Matrix3 curTM = oai->GetControllerTM(t);
			Matrix3 relTM = Inverse(curTM) * destTM;

			// Here's the modifications we need to make to get there
			Matrix3 tm;
			tm.SetTrans(curTM.GetTrans());

			AffineParts parts;
			decomp_affine(relTM, &parts);

			// Adjust the transform to express in obj space
			if (oai->HasObject())
				curTM *= oai->GetRefObjectTM(t) * iobtm;

			Point3 delta = destTM.GetTrans() - curTM.GetTrans();

			// Special rotation ignoring parent
			Matrix3 id;
			SetXFormPacket pckt(parts.q, FALSE, id, tm);
			oai->SetControllerPacket(t, pckt);

			// Move
			oai->Move(t, delta);
			break;
		}
	}

	nodeList.DisposeTemporary();

	return bFound;
}

int FaceAlignMouseProc::proc(HWND hWnd, int msg, int point, int flags, IPoint2 m)
{
	switch (msg)
	{
	case MOUSE_POINT:
		if (flags & MOUSE_LBUTTON)
			FaceAlignMap(hWnd, m, TRUE);
		RestoreMode();
		break;

	case MOUSE_MOVE:
	case MOUSE_FREEMOVE:
		if (FaceAlignMap(hWnd, m))
			SetCursor(ip->GetSysCursor(SYSCUR_SELECT));
		else
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		break;

	case MOUSE_ABORT:
	case MOUSE_PROPCLICK:
		RestoreMode();
		break;

	}
	return TRUE;
}

void FaceAlignMode::EnterMode()
{
	bActive = TRUE;
	if (oai)
		oai->EnterPick();
}

void FaceAlignMode::ExitMode()
{
	if(!bActive)
		return;

	bActive = FALSE;
	if (oai)
		oai->ExitPick();
}

void FaceAlignMode::AlignPressed()
{
	if (!bActive) {
		alignproc.StoreMode();
		ip->SetCommandMode(this);
	}
	else {
		ip->SetStdCommandMode(CID_OBJSELECT);
	}
}