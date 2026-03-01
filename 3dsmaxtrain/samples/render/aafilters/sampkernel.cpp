/**********************************************************************
 *<
	FILE: sampkernel.cpp	

	DESCRIPTION: Simple prefilter kernel....the cylinder of radius 0.5 pixels

	CREATED BY: Kells Elmquist

	HISTORY: created 7/29/98

 *>	Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/

#include "kernelhdr.h"
#include "kernelres.h"
//#include "imtl.h"
//#include <bmmlib.h>
#include "iparamm.h"

#define SAMPKERNEL_CLASS_ID 0x77912300

Class_ID sampKernelClassID(SAMPKERNEL_CLASS_ID ,0);
#define SAMPKERNEL_CLASSNAME   GetString(IDS_KE_SAMPKERNEL)


class SampKernelDlgProc; // forward

class SampleKernel: public FilterKernel {
	public:
		// Parameters
		IParamBlock *pblock;
		
		// Caches
		static SampKernelDlgProc *dlg;

		SampleKernel();
		~SampleKernel() { 	};

		// >>>> is a kernel w/ no params animatable ???

		// Animatable/Reference
		int NumSubs() {return 1;};
		Animatable* SubAnim(int i);
		TSTR SubAnimName(int i, bool localized) override;
		int NumRefs() {return 1;};
		RefTargetHandle GetReference(int i);
private:
		virtual void SetReference(int i, RefTargetHandle rtarg);
public:
		Class_ID ClassID() {return sampKernelClassID;};
		void GetClassName(MSTR& s, bool localized) const override {s = localized ? SAMPKERNEL_CLASSNAME : _T("Cylinder");};
		void DeleteThis() {delete this;};
		RefResult NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, 
	         PartID& partID, RefMessage message, BOOL propagate);

		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		TSTR GetName(bool localized) const override { return localized ? SAMPKERNEL_CLASSNAME : _T("Cylinder"); }
//		FilterKernelParamDlg *CreateParamDialog(IRendParams *ip);

		// there are 2 optional 0...1 parameters
		long GetNFilterParams() { return 0; }
		const TCHAR * GetFilterParamName( long nParam ) { return GetString( IDS_KE_BLANK );	}
		double GetFilterParam( long nParam ){ return 0.0; }
		void SetFilterParam( long nParam, double val ){}
		const TCHAR * GetDefaultComment() { return GetString( IDS_KE_BLANK); }

		double KernelFn( double x, double y );

		// integer number of pixels from center to filter 0 edge, must not truncate filter
		// x dimension for 2D filters
		long GetKernelSupport(){ return 1; }

		// for 2d returns y support, for 1d returns 0
		long GetKernelSupportY(){ return 0; }

		bool Is2DKernel(){ return FALSE; }
		bool IsVariableSz(){ return FALSE; }
		// 1-D filters ignore the y parameter, return it as 0.0
		void SetKernelSz( double x, double y = 0.0 ){}
		void GetKernelSz( double& x, double& y ){ x = 0.5; y = 0.0; }

		// returning true will disable the built-in normalizer
		bool IsNormalized(){ return FALSE; }

		// this is for possible future optimizations, not sure its needed
		bool HasNegativeLobes(){ return FALSE; }
	};

class SampKernelClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void*			Create(BOOL loading) { return new SampleKernel; }
	const TCHAR*	ClassName() { return SAMPKERNEL_CLASSNAME; }
	const TCHAR*	NonLocalizedClassName() { return _T("Cylinder"); }
	SClass_ID		SuperClassID() { return FILTER_KERNEL_CLASS_ID; }
	Class_ID 		ClassID() { return sampKernelClassID; }
	const TCHAR* 	Category() { return _T("");  }
};

static SampKernelClassDesc sampKernelCD;
ClassDesc* GetSampKernelDesc() {return &sampKernelCD;}

// Parameter Block
#define	PB_DUMMY_PARM	0

static ParamUIDesc descParam[] = {
	ParamUIDesc(PB_DUMMY_PARM , TYPE_SINGLECHECKBOX, 0 )
	
	};
#define PARAMDESC_LENGH 1

static ParamBlockDescID descVer0[] = {
	{ TYPE_INT, NULL, TRUE, 0 } // dummy param, for demonstration only
	}; 	
#define PBLOCK_LENGTH 1


#define NUM_OLDVERSIONS	0
#define CURRENT_VERSION	1

//static ParamVersionDesc curVersion(descVer0,PBLOCK_LENGTH,CURRENT_VERSION);
static ParamVersionDesc curVersion(NULL, 0, CURRENT_VERSION);

//--- SampleEffect ----------------------------------------------------------

//SampKernelDlgProc *SampleKernel::dlg = NULL;

SampleKernel::SampleKernel()
{
	pblock = NULL;
	ReplaceReference(0, CreateParameterBlock(descVer0, PBLOCK_LENGTH, CURRENT_VERSION));
	assert(pblock);
	pblock->SetValue(PB_DUMMY_PARM, 0, 0 );		
}

IOResult SampleKernel::Load(ILoad *iload)
{
	FilterKernel::Load(iload);
	return IO_OK;
}
IOResult SampleKernel::Save(ISave *isave)
{
	FilterKernel::Save(isave);
	return IO_OK;
}

//FilterKernelParamDlg *SampleKernel::CreateParamDialog(IRendParams *ip)
//{	
//	return new SampKernelParamDlg(this,ip);
//	return NULL;
//}

Animatable* SampleKernel::SubAnim(int i) 
{
	switch (i) {
		case 0: return pblock;
		default: return NULL;
	}
}

TSTR SampleKernel::SubAnimName(int i, bool localized)
{
	switch (i)
	{
	case 0: return localized ? GetString(IDS_KE_PARAMETERS) : _T("dummy parameter");
	default: return _T("");
	}
}

RefTargetHandle SampleKernel::GetReference(int i)
{
	switch (i) {
		case 0: return pblock;
		default: return NULL;
	}
}

void SampleKernel::SetReference(int i, RefTargetHandle rtarg)
{
	switch (i) {
		case 0: pblock = (IParamBlock*)rtarg; break;
	}
}

RefResult SampleKernel::NotifyRefChanged(
		const Interval& changeInt, RefTargetHandle hTarget,
		PartID& partID, RefMessage message, BOOL propagate) 
{
	GetParamName * gpn;

	switch (message) {
		case REFMSG_CHANGE:
			//valid.SetEmpty();
			//if (dlg)
			//	dlg->Invalidate();
			;
			break;

		case REFMSG_GET_PARAM_DIM: {
			GetParamDim * gpd = (GetParamDim*)partID;
			switch (gpd->index) {
				case PB_DUMMY_PARM: gpd->dim = defaultDim; break;
				default:            gpd->dim = defaultDim;
			}
			return REF_HALT;
		}

		case REFMSG_GET_PARAM_NAME_LOCALIZED:
		case REFMSG_GET_PARAM_NAME_NONLOCALIZED: {
			gpn = (GetParamName*)partID;
			switch (gpn->index) {
				case PB_DUMMY_PARM: gpn->name = _T("Dummy"); break;
				default:            gpn->name = _T("");      break;
			}
			return REF_HALT;
		}
	}
	return REF_SUCCEED;
}

double SampleKernel::KernelFn( double x, double y )
{
	return 	( x < 0.5 ) ? 1.0 : 0.0;
}

