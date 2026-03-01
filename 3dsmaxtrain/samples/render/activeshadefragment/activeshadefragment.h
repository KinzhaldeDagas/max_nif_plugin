//**************************************************************************/
// Copyright (c) 2014 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include <Graphics/FragmentGraph/ViewFragment.h>
#include <Graphics/ViewSystem/EvaluationContext.h>
#include <Graphics/BaseRasterHandle.h>
#include <Graphics/TargetHandle.h>
#include <interactiverender.h>
#include <Rendering/Renderer.h>
#include <vector>
#include <Graphics/ViewSystem/IActiveShadeFragment.h>
#include <Graphics/CustomRenderStageHandle.h>

namespace MaxGraphics 
{
	class ActiveShadeFragment : 
		public MaxSDK::Graphics::ViewFragment,
		public MaxSDK::Graphics::IActiveShadeFragment,
		virtual public IIRenderMgr, 
		virtual public IIRenderMgrSelector
	{
	public:
		ActiveShadeFragment();
		virtual ~ActiveShadeFragment();

		void DeleteThis() override { delete this; };
	
		Class_ID GetClassID() const override;

		//From MaxSDK::Graphics::IActiveShadeFragment
			/** Is ActiveShade enabled ?
			\return true if ActiveShade is running in this viewport, false if it is not running.
			*/
			bool IsEnabled() const override;

			/** Enable or disable ActiveShade in this viewport (meaning respectively run or stop ActiveShade in this viewport)
			\param[in] a boolean which is true to run ActiveShade in this viewport or false to stop it
			*/
			void Enable(bool val) override;

			/** Get the ActiveShade fragment resolution multiplier.
			A multiplier of 0.5 will give a half resolution ActiveShade rendering used and upscaled in the viewport. So a better time performance but resulting in a lower image quality
			This multiplier has to be a positive, non-zero float number.
			\return the value of the multiplier.
			*/
			float GetResolutionMultiplier() const override;

			/** Set the ActiveShade fragment resolution multiplier.
			A multiplier of 0.5 will give a half resolution ActiveShade rendering used and upscaled in the viewport. So a better time performance but resulting in a lower image quality
			This multiplier has to be a positive, non-zero float number.
			\param[in] the value of the multiplier, if the multipler is negative or 0, we set that value to 1.0f.
			*/
			void SetResolutionMultiplier(float factor) override;


			/**Set the ActiveShade renderer instance to use for running ActiveShade in this viewport. This has to be an ActiveShade compatible renderer (different from Scanline as it is not supported).
			If none is set, or the renderer instance set is not compatible with ActiveShade, we will use the ActiveShade renderer from the render settings.
			\param[in] an instance of an ActiveShade compatible renderer.
			*/
			void SetActiveShadeRenderer(Renderer* pRenderer) override;

			/**Get the ActiveShade renderer instance to use for running ActiveShade in this viewport. This can be an ActiveShade renderer taken from the render settings or
			an instance you have set using SetActiveShadeRenderer(Renderer* pRenderer).
			\return an ActiveShade renderer pointer used by this viewport if the ActiveShade fragment is enabled.
			*/
			Renderer* GetActiveShadeRenderer()const override;

	protected:
		
		//From ViewFragment
		bool DoEvaluate(MaxSDK::Graphics::EvaluationContext* evaluationContext) override;

		//From IIRenderMgr
		bool CanExecute() const override
		{
			return mIRenderInterface != nullptr;
		}
		void SetActive(bool /*active*/) override
		{
		}
		const MCHAR* GetName() const override
		{
			return _M("");
		}
		bool IsActive() const override
		{
			return true;
		}
		HWND GetHWnd() const override
		{
			return mHwnd;
		}
		ViewExp *GetViewExp() override
		{
			return mpViewExp;
		}
		void SetPos(int X, int Y, int W, int H) override
		{
			// do nothing.
		}
		void Show() override
		{
			// do nothing.
		}
		void Hide() override
		{
			// do nothing.
		}
		void UpdateDisplay() override;

		void Render() override
		{
			// do nothing.
		}
		void SetDelayTime(int msecDelay) override
		{
			// do nothing.
		}
		int GetDelayTime() override
		{
			return 100;
		}
		void Close() override
		{
			// do nothing.
		}
		void Delete() override
		{
			// do nothing.
		}
		void SetCommandMode(IIRenderMgr::CommandMode commandMode) override
		{
			// do nothing.
		}
		IIRenderMgr::CommandMode GetCommandMode() const override
		{
			return IIRenderMgr::CMD_MODE_NULL;
		}
		void SetActOnlyOnMouseUp(bool actOnlyOnMouseUp) override
		{
			// do nothing.
		}
		bool GetActOnlyOnMouseUp() const override
		{
			return true;
		}
		void ToggleToolbar() const override
		{
			// do nothing.
		}
		IImageViewer::DisplayStyle GetDisplayStyle() const override
		{
			return IImageViewer::IV_DOCKED;
		}
		BOOL IsRendering() override
		{
			if(mIRenderInterface == nullptr)
			{
				return FALSE;
			}
			return mIRenderInterface->IsRendering(); 
		}
		BOOL AreAnyNodesSelected() const override
		{
			return FALSE;
		}
		IIRenderMgrSelector* GetNodeSelector() override
		{
			return this;
		}
		BOOL AnyUpdatesPending() override
		{
			return TRUE;
		}

	protected:
		// from IIRenderMgrSelector
		BOOL IsSelected(INode* pINode) override
		{
			UNUSED_PARAM(pINode);
			return FALSE;
		}

	private:
		void Startup(MaxSDK::Graphics::EvaluationContext* evaluationContext, size_t width, size_t height);
		void Cleanup();

	private:
		bool	mEnabled						= false;
		bool	mbRunning						= false;
		Bitmap* mpBitmap						= nullptr;
		Renderer* mpRenderer					= nullptr;
		IInteractiveRender* mIRenderInterface	= nullptr;
		float mResolutionMultiplier				= 1.0f; //Is used to render ActiveShade at a different resolution than the viewport so you can have a lower quality but better speed performance

		//When mResolutionMultiplier != 1.0 we need to store locally a TargetHandle at the good size with mResolutionMultiplier applied on the orginal size
		//As when the size is different we used to do a SetSize on the ActiveShade target which was clearing it so, when there was no update it was black instead of showing the last result...
		MaxSDK::Graphics::TargetHandle			mDifferentResolutionTargetHandle;

		//Used when you switch the resolution multipler, there is a little interval when there is no update
		//where we could display the usual buffer (when x 1 res multiplier was set) which is wrong in that case, better display an empty buffer
		//until there is an ActiveShade Bitmap update
		bool mbDisplayEmptyBuffer				= false;

		HWND mHwnd								= nullptr;
		ViewExp* mpViewExp						= nullptr;;

		DefaultLight mDefaultLights[2];
		int mNumDefaultLights					= 0;

		bool SynchronizeToTarget(MaxSDK::Graphics::BaseRasterHandle& targetHandle, MaxSDK::Graphics::EvaluationContext* evaluationContext);
		void CopyBitmapDX11	(MaxSDK::Graphics::BaseRasterHandle& targetHandle);

		size_t			mBitmapWidth			= 0;
		size_t			mBitmapHeight			= 0;
		Box2			mRenderRegion;
	};
}