/*===========================================================================*\
 | 
 |  FILE:	wM3_cache.cpp
 |			Weighted Morpher for MAX R3
 |			MorphCache class
 | 
 |  AUTH:   Harry Denholm
 |			Copyright(c) Kinetix 1999
 |			All Rights Reserved.
 |
 |  HIST:	Started 3-10-98
 | 
\*===========================================================================*/

#include "wM3.h"
#include <tbb/parallel_for.h>
#include <tbb/task_group.h>
#include <atomic>
#include <cassert>

void TargetCache::Init(INode *nd)
{	
	if(!nd) return;
	
	ObjectState os = nd->EvalWorldState(GetCOREInterface()->GetTime());

	mNumPoints = os.obj->NumPoints();
	mTargetPoints.resize(mNumPoints);

	tbb::parallel_for(
			tbb::blocked_range<int>(0, mNumPoints, 5000),
			[this, &os](const tbb::blocked_range<int>& range) {
				for (int i = range.begin(); i < range.end(); ++i)
				{
					mTargetPoints[i] = os.obj->GetPoint(i);
				}
			},
			tbb::simple_partitioner{});
}

void TargetCache::Reset(void)
{
	mTargetPoints.clear();
	mTargetINode = NULL;
}

TargetCache::TargetCache(const TargetCache &tcache)
{
	mTargetINode = tcache.mTargetINode;
	mNumPoints = tcache.mNumPoints;
	mTargetPoints = tcache.mTargetPoints;
	mTargetPercent = tcache.mTargetPercent;
}

void TargetCache::operator=(const TargetCache &tcache)
{
	mTargetINode = tcache.mTargetINode;
	mNumPoints = tcache.mNumPoints;
	mTargetPoints = tcache.mTargetPoints;
}

void TargetCache::operator=(const morphChannel &mchan)
{
	mNumPoints = mchan.mNumPoints;
	mTargetPoints = mchan.Points();
	mTargetINode = mchan.mConnection;
}

IOResult TargetCache::Save(ISave* isave)
{
	ULONG nb;

	isave->BeginChunk(MR3_TARGETCACHE_POINTS);
	isave->Write(&mNumPoints,sizeof(long),&nb);
	for(long k=0; k<mNumPoints; k++) { isave->Write(&mTargetPoints[k],sizeof(Point3),&nb); }
	isave->EndChunk();

	isave->BeginChunk(MR3_PROGRESSIVE_TARGET_PERCENT);
	isave->Write(&mTargetPercent,sizeof(float),&nb);
	isave->EndChunk();

	return IO_OK;
}

IOResult TargetCache::Load(ILoad* iload)
{
	ULONG nb;
	IOResult res = IO_OK;
	long k;

	while (IO_OK==(res=iload->OpenChunk())) 
	{
		switch(iload->CurChunkID())  
		{
			case MR3_TARGETCACHE_POINTS:
				res = iload->Read(&mNumPoints,sizeof(long),&nb); 
				mTargetPoints.resize(mNumPoints);
				for(k=0; k<mNumPoints; k++) { 
					res = iload->Read(&mTargetPoints[k], sizeof(Point3),&nb);
					if (res!=IO_OK) return res;
				}
			break;

			case MR3_PROGRESSIVE_TARGET_PERCENT:
				res = iload->Read(&mTargetPercent,sizeof(float),&nb); 
			break;
		}

		iload->CloseChunk();
		if (res!=IO_OK) return res;
	}

	return IO_OK;
}

void morphCache::NukeCache()
{
	points_x.clear();
	points_y.clear();
	points_z.clear();
	weights.clear();
	sel.clear();
	selection_weights.clear();

	Count = 0;
	CacheValid = false;
}

void morphCache::MakeCache(Object* obj)
{
	assert(obj);
	if (obj == nullptr)
		return;

	assert(!AreWeCached());
	//if (AreWeCached())
	//{
	//	NukeCache();
	//}

	Count = obj->NumPoints();
	points_x.resize(Count);
	points_y.resize(Count);
	points_z.resize(Count);
	weights.resize(Count);
	sel.resize(Count);
	selection_weights.resize(Count);

	for (int t = 0; t < Count; t++)
	{
		const Point3& vert = obj->GetPoint(t);
		points_x[t] = vert.x;
		points_y[t] = vert.y;
		points_z[t] = vert.z;
		weights[t] = obj->GetWeight(t);
		selection_weights[t] = obj->PointSelection(t);
		sel[t] = selection_weights[t] > 0.f ? true : false;
	}

	CacheValid = true;
}

void morphCache::MakeSelectionCache(Object* obj)
{
	assert(obj);
	if (obj == nullptr)
		return;


	Count = obj->NumPoints();
	sel.resize(Count);
	selection_weights.resize(Count);

	for (int t = 0; t < Count; t++)
	{
		selection_weights[t] = obj->PointSelection(t);
		sel[t] = selection_weights[t] > 0.f ? true : false;
	}
}

bool morphCache::AreWeCached() const
{
	return CacheValid;
}
