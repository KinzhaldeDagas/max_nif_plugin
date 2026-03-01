/**********************************************************************
 *<
    FILE: tapehelp.h

    DESCRIPTION:  Defines a Measuring Tape Helper Class

    CREATED BY: Don Brittain

    HISTORY: created 8 October 1995

 *> Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#ifndef __TAPEHELP__H__
#define __TAPEHELP__H__

#include <float.h>
#define MIN_TAPE_LEN 0.0f
#define MAX_TAPE_LEN 999999.0f

class TapeHelpCreateCallBack;

class TapeHelpObject: public HelperObject
{
public:

    TapeHelpObject();
    ~TapeHelpObject();

    void                     SetLength(TimeValue t, float len);
    float                    GetLength(TimeValue t, Interval& valid);
    float GetLength(TimeValue t) { Interval valid(0,0); return GetLength(t, valid); }
    void                     SetSpecLen(int onOff);
    int                      GetSpecLen(void) { return specLenState; }
    void                     Enable(int enab) { enable = enab; }

    // Inherited virtual methods:

    // From BaseObject
    int                      HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt) override;
    void                     Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) override;
    int                      Display(TimeValue t, INode* inode, ViewExp *vpt, int flags) override;
    CreateMouseCallBack*     GetCreateMouseCallBack() override;
    void                     BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev) override;
    void                     EndEditParams(IObjParam *ip, ULONG flags,Animatable *next) override;
    const TCHAR *            GetObjectName(bool localized) const override { return localized ? GetString(IDS_DB_TAPE) : _T("Tape"); }

    // From Object
    ObjectState              Eval(TimeValue time) override;
    void                     InitNodeName(TSTR& s) override { s = GetString(IDS_DB_TAPE); }
    Interval                 ObjectValidity(TimeValue time) override;
    int                      DoOwnSelectHilite() override { return 1; }

    // From GeomObject
    int                      IntersectRay(TimeValue t, Ray& r, float& at);
    void                     GetWorldBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box) override;
    void                     GetLocalBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box) override;
    void                     GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel) override;

    BOOL                     HasViewDependentBoundingBox() override { return true; }

    // Animatable methods
    void                     DeleteThis() override { delete this; }
    Class_ID                 ClassID() override { return Class_ID(TAPEHELP_CLASS_ID,0); }
    void                     GetClassName(MSTR& s, bool localized) const override { s = localized ? GetString(IDS_DB_TAPEHELPER_CLASS) : _T("Tape"); }

    int                      NumSubs() override { return 1; }
    Animatable*              SubAnim(int i) override { return pblock; }
    TSTR SubAnimName(int i, bool localized) override { return localized ? GetString(IDS_RB_PARAMETERS) : _T("Parameters"); }

    // From ref
    RefTargetHandle          Clone(RemapDir& remap) override;
    int                      NumRefs() override { return 1; }
    RefTargetHandle          GetReference(int i) override { return pblock; }

    // IO
    IOResult                 Save(ISave *isave) override;
    IOResult                 Load(ILoad *iload) override;

private:

    friend class             TapeHelpObjCreateCallBack;
    friend INT_PTR CALLBACK  TapeHelpParamDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    friend void              resetTapeParams();

    // Class vars
    static Mesh              mesh;
    static short             meshBuilt;
    static HWND              hTapeHelpParams;
    static IObjParam *       iObjParams;
    static ISpinnerControl * lengthSpin;
    static float             dlgLength;
    static short             dlgSpecLen;

    // Object parameters
    IParamBlock *            pblock;
    short                    enable; // If true, hit testing is performed so that the Tape Helper can be selected by clicking on it
    short                    editting;
    short                    specLenState;
    float                    lastDist;
    Point3                   dirPt;

    Interval                 ivalid;

    // Inherited virtual methods for Reference-management
    RefResult                NotifyRefChanged(const Interval& changeInt,
                                              RefTargetHandle hTarget,
                                              PartID&         partID,
                                              RefMessage      message,
                                              BOOL            propagate) override;

    void                     BuildMesh();
    void                     UpdateUI(TimeValue t);
    void                     GetMat(TimeValue t, INode* inod, ViewExp &vpt, Matrix3& mat);
    void                     GetLinePoints(TimeValue t, Point3* q, float len);
    int                      DrawLine(TimeValue t, INode* inode, GraphicsWindow *gw, int drawing);

    virtual void             SetReference(int i, RefTargetHandle rtarg) override { pblock = (IParamBlock*)rtarg; }
};

#endif
