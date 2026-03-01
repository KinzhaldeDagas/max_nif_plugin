//**************************************************************************/
// Copyright (c) 2017 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

//From Max SDK
#include <iparamb2.h>
#include <Graphics/ViewSystem/EvaluationContext.h>
#include <Rendering/IActiveShadeFragmentManager.h>
#include <Graphics/DeviceCaps.h>
#include <Graphics/IDisplayManager.h>
#include <ColorManagement/IColorPipelineMgr.h>

// TBB
#include <tbb/parallel_for.h>
#include <tbb/task_arena.h>
// Imath
#include <Imath/half.h>

//Local headers
#include "activeshadefragment.h"
#include "Resource.h"
#include "gamma.h"

extern HINSTANCE hInstance;

using namespace MaxSDK::Graphics;
using namespace MaxSDK::ColorManagement;

#define ACTIVE_SHADE_FRAGMENT_CLASS_ID Class_ID(0x12c31f85, 0xe40a704e)

namespace {
	// Determines the size of the safe frame window
	static IPoint2 GetSafeFrameWindowSize(const IPoint2& viewportSize)
	{
		// The safe Frame is as big as it can be inside the current window and is always in the middle.
		// Try to think of putting the biggest rectangle you can inside another while maintaining the
		// aspect ratio of the rectangle you're inserting, so there are two possible scenarios:
		//
		//            W = SFW                                   W 
		//      <-------------------->                <-------------------->
		//    ^ ......................                ......................  ^
		//    | :                    :	              :    :          :    :  |
		//    | :....................: ^              :    :          :    :  |
		//    | :                    : |              :    :          :    :  |
		//  H | :                    : | SFH          :    :          :    :  | H = SFH
		//    | :                    : |              :    :          :    :  |
		//    | :....................: v              :    :          :    :  |
		//    | :                    :                :    :          :    :  |
		//    v :....................:                :....................:  v
		//                                                 <--------->
		//                                                    SFW
		//												In that case the FOV is adjusted in the viewport/camera (see the function Max::NotificationAPI::Utils::GetFOVFromViewExp)
		//	  W: Width of Viewport
		//    H: Height of Viewport                  
		//  SFH: Height of Safe Frame Window       
		//	SFW: Width of Safe Frame Window
		//											


		// Get the aspect ratios to see in which scenario we are
		const float safeFrameImgAspect	= GetCOREInterface()->GetRendImageAspect();
		const float vptAspect			= float(viewportSize.y)/float(viewportSize.x);

		// So now we can calculate the size 
		IPoint2 safeFrameSize;

		// Try it one way
		safeFrameSize.x = viewportSize.x;
		safeFrameSize.y = safeFrameSize.x/safeFrameImgAspect;

		// Check that we haven't overflowed, if we have then do it the other way
		if (safeFrameSize.x > viewportSize.x || safeFrameSize.y > viewportSize.y)
		{
			safeFrameSize.y = viewportSize.y;
			safeFrameSize.x = safeFrameSize.y*safeFrameImgAspect;
		}

		return safeFrameSize;
	}
};

class ActiveShadeFragmentDesc:public ClassDesc2
{
public:
	int 			IsPublic() override {return TRUE;}
	void*			Create(BOOL /*loading*/) override
	{
		//Cast it into a Fragment* as it is going to be returned as a void* then casted 
		//after into a Fragment* and that breaks the vtable as this is not only a Fragment instance
		Fragment* pFragment = new MaxGraphics::ActiveShadeFragment;
		return pFragment;
	}
	const MCHAR*	ClassName() override { return _M("ActiveShadeFragment"); }
	const MCHAR*	NonLocalizedClassName() override { return _M("ActiveShadeFragment"); }
	SClass_ID		SuperClassID() override { return Fragment_CLASS_ID; }
	Class_ID		ClassID() override { return ACTIVE_SHADE_FRAGMENT_CLASS_ID;}
	const MCHAR* 	Category() override { return _M("Fragment"); }

	const TCHAR*	InternalName() override { return _T("ActiveShadeFragment"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() override { return hInstance; }				// returns owning module handle
	bool			UseOnlyInternalNameForMAXScriptExposure() override { return true; }
};
static ActiveShadeFragmentDesc oneActiveShadeFragmentDesc;
ClassDesc2* GetActiveShadeFragmentDesc() { return &oneActiveShadeFragmentDesc; }

namespace MaxGraphics 
{
	enum ActiveShadeFragmentInput
	{
		ActiveShadeFragmentInput_ColorTarget, 
		ActiveShadeFragmentInput_Count, 
	};

	enum ActiveShadeFragmentOutput
	{
		ActiveShadeFragmentOutput_ColorTarget, 
		ActiveShadeFragmentOutput_Count, 
	};

	ActiveShadeFragment::ActiveShadeFragment()
	{
		mpBitmap = nullptr;
		mpRenderer = nullptr;
		mIRenderInterface = nullptr;

		mHwnd = nullptr;
		mpViewExp = nullptr;
		memset(mDefaultLights, 0, _countof(mDefaultLights) * sizeof(DefaultLight));
		mNumDefaultLights = 0;

		mBitmapWidth				= 1;
		mBitmapHeight				= 1;

		Class_ID inputIDs[ActiveShadeFragmentInput_Count];
		inputIDs[ActiveShadeFragmentInput_ColorTarget] = TargetHandle::ClassID();
		InitializeInputs(ActiveShadeFragmentInput_Count, inputIDs);

		Class_ID outputIDs[ActiveShadeFragmentOutput_Count];
		outputIDs[ActiveShadeFragmentOutput_ColorTarget] = TargetHandle::ClassID();
		InitializeOutputs(ActiveShadeFragmentOutput_Count, outputIDs);
	}

	ActiveShadeFragment::~ActiveShadeFragment()
	{
		Cleanup();
	}

	bool ActiveShadeFragment::IsEnabled() const
	{
		return mEnabled;
	}

	void ActiveShadeFragment::Enable(bool val)
	{
		//Mark view as dirty to make redraw take effect
		if (mEnabled != val) {
			if (Interface* ip = GetCOREInterface()) {
				ViewExp& vp = ip->GetViewExp(mHwnd);
				if (vp.IsAlive()) {
					if (ViewExp10* vp10 = reinterpret_cast<ViewExp10*>(vp.Execute(ViewExp::kEXECUTE_GET_VIEWEXP_10))) {
						vp10->Invalidate();
					}
				}
			}
		}

		if (false == val && mbRunning) {
			Cleanup();//We are disabling it but it was running, so end interactive session
			SetSmartHandleOutput(EmptyTargetHandle, ActiveShadeFragmentOutput_ColorTarget); //Clear the output buffer
		}

		mEnabled = val;
	}

	bool ActiveShadeFragment::DoEvaluate(EvaluationContext* evaluationContext)
	{
		if (false == mEnabled) 
		{
			return SetSmartHandleOutput(EmptyTargetHandle, ActiveShadeFragmentOutput_ColorTarget);
		}

		if(nullptr == evaluationContext)
		{
			return false;
		}

		ViewParameter* pViewportParameter = evaluationContext->pViewParameter.GetPointer();

		if(pViewportParameter->IsRenderRegionVisible())
		{
			mBitmapWidth	= pViewportParameter->GetWidth();
			mBitmapHeight	= pViewportParameter->GetHeight();
			const IPoint2 viewportSize((int)mBitmapWidth, (int)mBitmapHeight);

			//Apply multiplier
			DbgAssert(mResolutionMultiplier > 0.f);
			
			//When safe frame is on, some differences should be taken into account
			bool bSafeFrameIsOn = false;
			ViewExp* pViewExp = pViewportParameter->GetViewExp();
			if (nullptr != pViewExp){
				bSafeFrameIsOn = (1 == pViewExp->getSFDisplay());
			}
			
			const RenderRegion& rr			= pViewportParameter->GetRenderRegion();
			
			//rr.mScreenSpaceScale is (1.0, 1.0) when not using safe frame and if no one has modified the scale of the render region
			//But one of its component is non-unit when using safe frame
			const float fBitmapWidth		= mResolutionMultiplier * (float)mBitmapWidth  * rr.mScreenSpaceScale.x;
			const float fBitmapHeight		= mResolutionMultiplier * (float)mBitmapHeight * rr.mScreenSpaceScale.y;
			
			mBitmapWidth					= floorf(fBitmapWidth);//This is taking into account any potential safe frame
			mBitmapHeight					= floorf(fBitmapHeight);

			if (bSafeFrameIsOn){
			
				const IPoint2 sfWindowSize		= GetSafeFrameWindowSize(viewportSize);
			
				//When safe frame is on, we should keep the same aspect ratio that was before in the full size viewport. 
				//In one of the case (sfWindowSize.y == viewportSize.y), safe frame also modifies the FOV which is handled in the Notification and Rendering APIs from Max SDK
				if (sfWindowSize.x == viewportSize.x){
					mBitmapHeight = mBitmapHeight / rr.mScreenSpaceScale.y;
				}else{
					mBitmapHeight = mBitmapHeight * rr.mScreenSpaceScale.x;
				}

				mRenderRegion.left		= (mBitmapWidth-1)  * rr.mPostRenderClippingRegion.pmin.x;//mBitmapWidth/Height already include the resolution multiplier
				mRenderRegion.top		= (mBitmapHeight-1) * rr.mPostRenderClippingRegion.pmin.y;
				mRenderRegion.right		= (mBitmapWidth-1)  * rr.mPostRenderClippingRegion.pmax.x;
				mRenderRegion.bottom	= (mBitmapHeight-1) * rr.mPostRenderClippingRegion.pmax.y;
			}else{
				const FBox2& clippedTargetRegion = pViewportParameter->GetClippedTargetRegion();

				mRenderRegion.left		= std::clamp(int((float)(clippedTargetRegion.pmin.x) * mResolutionMultiplier), 0, ((int)mBitmapWidth-1));
				mRenderRegion.top		= std::clamp(int((float)(clippedTargetRegion.pmin.y) * mResolutionMultiplier), 0, ((int)mBitmapHeight-1));
				mRenderRegion.right		= std::clamp(int((float)(clippedTargetRegion.pmax.x) * mResolutionMultiplier), 0, ((int)mBitmapWidth-1));
				mRenderRegion.bottom	= std::clamp(int((float)(clippedTargetRegion.pmax.y) * mResolutionMultiplier), 0, ((int)mBitmapHeight-1));
			}

			TargetHandle targetHandle;
			if (!GetSmartHandleInput(targetHandle,ActiveShadeFragmentInput_ColorTarget) ||
				!targetHandle.IsValid())
			{
				return false;
			}

			if (SynchronizeToTarget(targetHandle, evaluationContext))
			{
				if (mbDisplayEmptyBuffer){
					return SetSmartHandleOutput(EmptyTargetHandle, ActiveShadeFragmentOutput_ColorTarget);
				}

				return SetSmartHandleOutput(targetHandle, ActiveShadeFragmentOutput_ColorTarget);
			}
		}

		return false;	
	}

	bool ActiveShadeFragment::SynchronizeToTarget(BaseRasterHandle& targetHandle,
		EvaluationContext* evaluationContext)
	{
		if (nullptr == mIRenderInterface) //Not yet taken the mIRenderInterface so call startup
		{
			Startup(evaluationContext, mBitmapWidth, mBitmapHeight);
		}

		if (nullptr == mIRenderInterface)
		{
			DbgAssert(0);
			return false;
		}

		if (mpBitmap == nullptr || 
			mpBitmap->Width() != mBitmapWidth || 
			mpBitmap->Height() != mBitmapHeight)
		{
			// Viewport resolution has changed like resizing or maximizing the
			// viewport
			Cleanup();
			Startup(evaluationContext, mBitmapWidth, mBitmapHeight);
		}

		if (targetHandle.GetWidth() != mBitmapWidth ||
			targetHandle.GetHeight() != mBitmapHeight)
		{
			if (false == mDifferentResolutionTargetHandle.IsValid())
			{
				// We can't change the resolution of the original ActiveShade
				// targetHandle which comes from the view graph creation.  so we
				// need to store locally a new TargetHandle which has a
				// different resolution (mDifferentResolutionTargetHandle) and
				// use this for the copy.  When the resolution changes, this
				// mDifferentResolutionTargetHandle is released, so we just need
				// to initialize it afterwards
				mDifferentResolutionTargetHandle.Initialize(mBitmapWidth, mBitmapHeight, targetHandle.GetFormat());
			}
			
			// Make current targethandle point on the
			// mDifferentResolutionTargetHandle which has the same resolution as
			// ActiveShade bitmap So when there is no update it keeps what was
			// drawn before and avoids a targetHandle.Setsize(...) as it was
			// before which was clearing the buffer.
			targetHandle = mDifferentResolutionTargetHandle;
		}

		// ColorManagement
		if (GetIDisplayManager()->IsViewportColorPipelineOCIOBased())
		{
			TargetFormat format = GetIDisplayManager()->GetViewportColorPipelineTargetFormat() == ColorPipelineTargetFormat::kFormat_FLOAT_16 ? TargetFormatA16B16G16R16F : TargetFormatA32B32G32R32F;
			if (targetHandle.GetFormat() != format)
			{
				targetHandle.SetFormat(format);
			}
		}
		else
		{
			if (targetHandle.GetFormat() != TargetFormatA8R8G8B8)
			{
				targetHandle.SetFormat(TargetFormatA8R8G8B8);
			}
		}

		// This flag is updated when Bitmap::PutPixels is called, so if it was
		// the case since last call we need to update the raster, if not, then
		// skip this
		if (0 == (mpBitmap->Flags() & MAP_WAS_UPDATED))
		{
			return true; // The Bitmap has not received any updates so ignore the copy into the raster
		}

		mbDisplayEmptyBuffer = false;//We have some updates now, no need to display an empty buffer

		CopyBitmapDX11(targetHandle);

		//Set the bitmap is up to date
		mpBitmap->ClearFlag(MAP_WAS_UPDATED);

		return true;
	}

	void ActiveShadeFragment::CopyBitmapDX11(BaseRasterHandle& targetHandle)
	{
		// ColorManagement
		auto format = targetHandle.GetFormat();
		if (format == TargetFormatA32B32G32R32F)
		{
			const int actualWidth = mRenderRegion.w();
			DbgAssert(actualWidth > 0);

			const int nLineWidth = actualWidth;

			// update color target!
			size_t destRowPitch = 0, destSlicePitch = 0;
			byte* pDestData = (byte*)targetHandle.Map(WriteAcess, destRowPitch, destSlicePitch);

			//Set pDestData on current line
			tbb::task_arena arena;
			auto lineBuffer = std::make_unique<BMM_Color_fl[]>(mBitmapWidth * arena.max_concurrency());
			tbb::parallel_for(tbb::blocked_range<int>(mRenderRegion.top, mRenderRegion.bottom), [&](const auto& range)
				{
					BMM_Color_fl* buffer = lineBuffer.get() + (mBitmapWidth * tbb::this_task_arena::current_thread_index());
					byte* pCurrDestData = pDestData + destRowPitch * range.begin();
					for (int y = range.begin(); y <= range.end(); ++y)
					{
						mpBitmap->GetPixels(mRenderRegion.left, y, nLineWidth, buffer + mRenderRegion.left);
						memcpy_s(pCurrDestData, destRowPitch, buffer + mRenderRegion.left, destRowPitch);
						pCurrDestData += destRowPitch;
					}
				}
			);
			//Set the bitmap is up to date
			mpBitmap->ClearFlag(MAP_WAS_UPDATED);

			targetHandle.UnMap();
		}
		else if (format == TargetFormatA16B16G16R16F)
		{
			const int actualWidth = mRenderRegion.w();
			DbgAssert(actualWidth > 0);

			const int nLineWidth = actualWidth;

			// update color target!
			size_t destRowPitch = 0, destSlicePitch = 0;
			byte* pDestData = (byte*)targetHandle.Map(WriteAcess, destRowPitch, destSlicePitch);

			// 16bit float type
			struct BMM_Color_fl_16 { half r,g,b,a; };

			//Set pDestData on current line
			tbb::task_arena arena;
			auto lineBuffer = std::make_unique<BMM_Color_fl[]>(mBitmapWidth * arena.max_concurrency());
			auto lineBuffer_half = std::make_unique<BMM_Color_fl_16[]>(mBitmapWidth * arena.max_concurrency());
			tbb::parallel_for(tbb::blocked_range<int>(mRenderRegion.top, mRenderRegion.bottom), [&](const auto& range)
				{
					BMM_Color_fl* buffer = lineBuffer.get() + (mBitmapWidth * tbb::this_task_arena::current_thread_index());
					BMM_Color_fl_16* buffer_half = lineBuffer_half.get() + (mBitmapWidth * tbb::this_task_arena::current_thread_index());
					byte* pCurrDestData = pDestData + destRowPitch * range.begin();
					for (int y = range.begin(); y <= range.end(); ++y)
					{
						mpBitmap->GetPixels(mRenderRegion.left, y, nLineWidth, buffer + mRenderRegion.left);
						// Float to Half conversion
						for (int x = mRenderRegion.left; x < mRenderRegion.right; ++x)
						{
							BMM_Color_fl_16 c_half;
							const BMM_Color_fl& c = buffer[x];
							c_half.r = c.r;
							c_half.g = c.g;
							c_half.b = c.b;
							c_half.a = c.a;
							buffer_half[x] = c_half;
						}
						memcpy_s(pCurrDestData, destRowPitch, buffer_half + mRenderRegion.left, destRowPitch);
						pCurrDestData += destRowPitch;
					}
				}
			);

			//Set the bitmap is up to date
			mpBitmap->ClearFlag(MAP_WAS_UPDATED);

			targetHandle.UnMap();
		}
		else
		{
			const int actualWidth = mRenderRegion.w();
			DbgAssert(actualWidth > 0);

			const int nLineWidth = actualWidth;

			// update color target!
			size_t destRowPitch = 0, destSlicePitch = 0;
			byte* pDestData = (byte*)targetHandle.Map(WriteAcess, destRowPitch, destSlicePitch);

			//Set pDestData on current line
			tbb::task_arena arena;
			auto lineBuffer = std::make_unique<BMM_Color_64[]>(mBitmapWidth * arena.max_concurrency());
			tbb::parallel_for(tbb::blocked_range<int>(mRenderRegion.top, mRenderRegion.bottom), [&](const auto& range)
				{
					BMM_Color_64* buffer = lineBuffer.get() + (mBitmapWidth * tbb::this_task_arena::current_thread_index());
					byte* pCurrDestData = pDestData + destRowPitch * range.begin();
					for (int y = range.begin(); y <= range.end(); ++y)
					{
						mpBitmap->GetPixels(mRenderRegion.left, y, nLineWidth, buffer + mRenderRegion.left);
						for (int x = mRenderRegion.left; x <= mRenderRegion.right; ++x)
						{
							const BMM_Color_64& LineColor = buffer[x];
							const DWORD color =
								(((DWORD)(LineColor.a >> 8)) << 24) |
								(((DWORD)(GAMMA16to8(LineColor.b)))) |
								(((DWORD)(GAMMA16to8(LineColor.g))) << 8) |
								(((DWORD)(GAMMA16to8(LineColor.r))) << 16);
							*((DWORD*)pCurrDestData + x) = color;//ABGR
						}
						pCurrDestData += destRowPitch;
					}
				}
			);

			//Set the bitmap is up to date
			mpBitmap->ClearFlag(MAP_WAS_UPDATED);

			targetHandle.UnMap();
		}
	}
	
	void ActiveShadeFragment::Startup(EvaluationContext* evaluationContext, size_t width, size_t height)
	{
		Interface* ip = GetCOREInterface();
		if (!ip)
		{
			return;
		}

		Renderer* pRenderer = mpRenderer;
		if (!pRenderer)
		{
			// No renderer has been set so use the one from the render settings
			pRenderer = ip->GetRenderer(RS_IReshade);
		}

		mpRenderer = (pRenderer) ? (Renderer*)CloneRefHierarchy(pRenderer) : nullptr;
		if (!mpRenderer)
		{
			DbgAssert(0 && _T("nullptr == mpRenderer"));
			return;
		}

		mIRenderInterface = mpRenderer->GetIInteractiveRender();
		if (nullptr == mIRenderInterface)
		{
			DbgAssert(0 && _T("Renderer set in fragment is not compatible with active shade"));
			return; // not an active shade renderer
		}

		// ColorManagement
		bool isOCIOMode = GetIDisplayManager()->IsViewportColorPipelineOCIOBased();

		// create the bitmap
		BitmapInfo bi;
		bi.SetWidth((int)width);
		bi.SetHeight((int)height);
		if (isOCIOMode)
		{
			bi.SetType(BMM_FLOAT_RGBA_32); // Use 32f format to avoid bits clipping
		}
		else
		{
			bi.SetType(BMM_TRUE_64); // 64-bit color: 16 bits each for Red, Green, Blue, and Alpha.
		}
		bi.SetFlags(MAP_HAS_ALPHA);
		bi.SetAspect(1.0f);
		mpBitmap = TheManager->Create(&bi);

		ViewParameter* pViewportParameter = evaluationContext->pViewParameter.GetPointer();

		mpViewExp = pViewportParameter->GetViewExp();
		mHwnd = mpViewExp->GetHWnd();

		// Add default lights to Active shade
		mNumDefaultLights = GetCOREInterface7()->InitDefaultLights(mDefaultLights, 2, FALSE, mpViewExp);
		mIRenderInterface->SetDefaultLights(mDefaultLights, mNumDefaultLights);

		mIRenderInterface->SetOwnerWnd( mpViewExp->GetHWnd() );
		mIRenderInterface->SetIIRenderMgr(this);
		mIRenderInterface->SetBitmap( mpBitmap );
		mIRenderInterface->SetSceneINode( ip->GetRootNode() );
		mIRenderInterface->SetViewExp(mpViewExp);

		// Set the active shade manager as the Progress Callback for this active shade fragment
		if (auto pActiveShadeFragmentManager = static_cast<MaxSDK::IActiveShadeFragmentManager*>(
					GetCOREInterface(IACTIVE_SHADE_VIEWPORT_MANAGER_INTERFACE)))
		{
			IRenderProgressCallback* pProgressCallback =
					pActiveShadeFragmentManager->GetActiveShadeFragmentProgressCallback(this);
			DbgAssert(pProgressCallback);
			mIRenderInterface->SetProgressCallback(pProgressCallback);
		}

		mIRenderInterface->SetRegion(mRenderRegion);
		mIRenderInterface->BeginSession();

		mbDisplayEmptyBuffer = true;//display an empty buffer until we get updates
		mbRunning = true;
	}

	void ActiveShadeFragment::Cleanup()
	{
		if (mIRenderInterface)
		{
			mIRenderInterface->EndSession();
		}

		if (auto pActiveShadeFragmentManager = static_cast<MaxSDK::IActiveShadeFragmentManager*>(
					GetCOREInterface(IACTIVE_SHADE_VIEWPORT_MANAGER_INTERFACE)))
		{
			pActiveShadeFragmentManager->RemoveActiveShadeFragmentProgressCallback(this);
		}
		
		if (nullptr != mpRenderer)
		{
			if (nullptr != mIRenderInterface)
			{
				// we're done with the reshading interface
				mIRenderInterface = nullptr;
			}
			mpRenderer->MaybeAutoDelete();
			mpRenderer = nullptr;
		}

		if (nullptr != mpBitmap)
		{
			mpBitmap->DeleteThis();
			mpBitmap = nullptr;
		}

		mbRunning = false;
		mDifferentResolutionTargetHandle.Release();
	}

	Class_ID ActiveShadeFragment::GetClassID() const
	{
		return ACTIVE_SHADE_FRAGMENT_CLASS_ID;
	}

	void ActiveShadeFragment::UpdateDisplay()
	{
		if (IsRendering())
		{
			SetFlag(FragmentFlagsDirty, true);
		}
	}

	void ActiveShadeFragment::SetActiveShadeRenderer(Renderer* pRenderer)
	{
		DbgAssert(pRenderer);
		mpRenderer = pRenderer;
	}

	Renderer* ActiveShadeFragment::GetActiveShadeRenderer() const
	{
		return mpRenderer;
	}

	float ActiveShadeFragment::GetResolutionMultiplier() const
	{
		return mResolutionMultiplier;
	}

	void ActiveShadeFragment::SetResolutionMultiplier(float factor)
	{
		if (fabsf(factor - mResolutionMultiplier) > 1.0e-3f) {
			//The value is actually changing
			mResolutionMultiplier = factor;
			if (mResolutionMultiplier <= 0) {
				mResolutionMultiplier = 1.0f;
			}

			Cleanup();//stop ActiveShade, it will be restarted at the good resolution later in SynchronizetoTarget
			mbDisplayEmptyBuffer = true;
		}
	}
}


