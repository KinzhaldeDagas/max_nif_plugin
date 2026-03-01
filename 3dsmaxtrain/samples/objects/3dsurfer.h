// 3dsurfer.h

#include "istdplug.h"

struct SurferPatchDataReaderCallback : public ObjectDataReaderCallback
{
	const char* DataName() const override { return "MinervaSoftware_Patch_3"; }
	Object* ReadData(TriObject *tri, void* data, DWORD len) override;
	void DeleteThis() override {}
};

struct SurferSplineDataReaderCallback : public ObjectDataReaderCallback
{
	const char* DataName() const override { return "MinervaSoftware_Spline_2"; }
	Object* ReadData(TriObject* tri, void* data, DWORD len) override;
	void DeleteThis() override {}
};
