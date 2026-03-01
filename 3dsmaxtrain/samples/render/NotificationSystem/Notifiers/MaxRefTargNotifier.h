//**************************************************************************/
// Copyright (c) 2013 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include "Notifiers/MaxNotifier.h"

namespace Max{

namespace NotificationAPI{

	class MaxRefTargNotifier : public MaxNotifier
	{
	public:
		explicit MaxRefTargNotifier(ReferenceTarget& RefTarg);

		ReferenceTarget* GetMaxRefTarg() const;

        //From MaxNotifier
        virtual void DebugPrintToFile(FILE* f, size_t indent)const;

        //-- From ReferenceMaker
		virtual RefResult NotifyRefChanged(
			const Interval& changeInt, 
			RefTargetHandle hTarget, 
			PartID& partID, 
			RefMessage message,
			BOOL propagate );
		virtual int NumRefs();
		virtual RefTargetHandle GetReference(int i);

		//-- From Animatable
		virtual void GetClassName(MSTR& s, bool localized) const override
		{
			UNUSED_PARAM(localized);
			s = _M("Max::NotificationAPI::MaxRefTargNotifier");
		}


    protected:

        // Protected destructor forces going through delete_this()
        virtual ~MaxRefTargNotifier();

	private:
		ReferenceTarget* m_refTarg;
        //from ReferenceMaker
        virtual void SetReference(int , RefTargetHandle rtarg);
	};

} } // namespaces
