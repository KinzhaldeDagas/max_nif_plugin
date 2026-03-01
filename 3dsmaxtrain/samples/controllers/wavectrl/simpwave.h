//**************************************************************************/
// Copyright (c) 2014 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#ifndef __SIMPWAVE__H
#define __SIMPWAVE__H

#define FLOATWAVE_CONTROL_CLASS_ID  0x930Abc78

class BaseWaveControlInternal :public LockableStdControl {
public:
	virtual int		GetWaveformsCount() {return 0;}
	virtual void	SetWaveformsCount(int iCount){}
};

#endif
