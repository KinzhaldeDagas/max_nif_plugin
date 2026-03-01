/*===========================================================================*\
 | 
 |  FILE:	wM3_channel.cpp
 |			Weighted Morpher for MAX R3
 |			Stuff for channel management
 | 
 |  AUTH:   Harry Denholm
 |			Copyright(c) Kinetix 1999
 |			All Rights Reserved.
 |
 |  HIST:	Started 15-10-98
 | 
\*===========================================================================*/

#include "wM3.h"

#include <tbb/parallel_for.h>
#include <tbb/task_group.h>
#include <atomic>

// Construct the class with defaults
morphChannel::morphChannel()
{
	mp = NULL;
	mNumPoints = 0;

	mConnection = NULL;
	mSel.ClearAll();

	// default name '-empty-'
	mName = GetString(IDS_EMPTY_CHANNEL);
	mNonLocalizedName = _T("- empty -");
	// channel starts 'inactive'
	mActive = FALSE;
	// channel value limits defaults
	mSpinmin = 0.0f;
	mSpinmax = 100.0f;
	mUseLimit = FALSE;
	mUseSel = FALSE;
	cblock = NULL;
	mModded = FALSE;
	mInvalid = FALSE;
	mActiveOverride = TRUE;
	mTargetCaches.resize(MAX_PROGRESSIVE_TARGETS);
	mNumProgressiveTargs=0;
	mCurvature=0.5f;
	iTargetListSelection = 0;
	mTargetPercent = 0.0f;
}

morphChannel::~morphChannel()
{
	mConnection = NULL;
	cblock = NULL;
	mSel.ClearAll();
}

morphChannel::morphChannel(const morphChannel & from)
{
	mNumPoints = from.mNumPoints;

	mPoints = from.mPoints;
	mDeltas = from.mDeltas;
	mWeights = from.mWeights;

	mTargetCaches = from.mTargetCaches;
	mSel = from.mSel;

	mActive = from.mActive;
	mActiveOverride = from.mActiveOverride;
	mConnection = from.mConnection;
	mCurvature = from.mCurvature;
	mInvalid = from.mInvalid;
	mModded = from.mModded;
	mName = from.mName;
	mNonLocalizedName = from.mNonLocalizedName;
	mNumProgressiveTargs = from.mNumProgressiveTargs;
	mUseLimit = from.mUseLimit;
	mUseSel = from.mUseSel;
	mSpinmin = from.mSpinmin;
	mSpinmax = from.mSpinmax;
	//not sure why this was left out
	cblock = from.cblock;

	mp = from.mp;
	mTargetPercent = from.mTargetPercent;
	iTargetListSelection = from.iTargetListSelection;
}

void morphChannel::InitTargetCache(const int& targnum, INode * nd)
{
	mTargetCaches[targnum].Init(nd);
}

void morphChannel::ResetMe()
{
	mPoints.clear();  mPoints.resize(MAX_PROGRESSIVE_TARGETS);
	mWeights.clear(); mWeights.resize(MAX_PROGRESSIVE_TARGETS);
	mDeltas.clear();  mDeltas.resize(MAX_PROGRESSIVE_TARGETS);
	mNumPoints = 0;
	mConnection = NULL;
	mName = GetString(IDS_EMPTY_CHANNEL);
	mNonLocalizedName = _T("- empty -");
	mActive = FALSE;
	mSpinmin = 0.0f;
	mSpinmax = 100.0f;
	mUseLimit = FALSE;
	mUseSel = FALSE;
	mModded = FALSE;
	mInvalid = FALSE;
	mActiveOverride = TRUE;
	mNumProgressiveTargs = 0;
	mCurvature = 0.5f;
	ReNormalize();
}

void morphChannel::AllocBuffers(int sizeA, int sizeB)
{
	tbb::task_group g;
	g.run([&, this]() {
		mPoints.clear();
		mPoints.resize(sizeA);
	});
	g.run([&, this]() {
		mDeltas.clear();
		mDeltas.resize(sizeA);
	});
	g.run_and_wait([&, this]() {
		mWeights.clear();
		mWeights.resize(sizeA);
	});
}


// Do some rough calculations about how much space this channel
// takes up
// This isn't meant to be fast or terribly accurate!
float morphChannel::getMemSize()
{
	float msize  = sizeof(*this);
	float pointsize = sizeof(Point3);
	msize += (pointsize*mNumPoints*(mNumProgressiveTargs+1));	// delta points
	msize += (sizeof(double)*mNumPoints);	// Weighting points
	return msize;
}

void morphChannel::operator=(const TargetCache& tcache)
{
	mNumPoints = tcache.mNumPoints;
	mPoints = tcache.mTargetPoints;
	mConnection = tcache.mTargetINode;
}

void morphChannel::ResetRefs(MorphR3 *mp, const int &cIndex)
{
	int refIDOffset = mp->GetRefIDOffset(cIndex);
	mp->ReplaceReference(101+cIndex%100+refIDOffset, mConnection);

	int refnum;
	for(int i=1; i<=mNumProgressiveTargs; i++) {
		refnum = 200 + (cIndex%100 * MAX_PROGRESSIVE_TARGETS) + i;
		mp->ReplaceReference(refnum + refIDOffset, mTargetCaches[i - 1].mTargetINode);
	}
}

void morphChannel::CopyTargetPercents(const morphChannel &chan)
{
	if(!&chan) return;
	mTargetPercent = chan.mTargetPercent;
	int targsize = static_cast<int>(mTargetCaches.size());	// SR DCAST64: Downcast to 2G limit.
	if( targsize!= chan.mTargetCaches.size() ) return;

	std::vector<TargetCache>& target_caches = mTargetCaches;
	for (int i = 0; i < targsize; i++)
	{
		target_caches[i].mTargetPercent = chan.mTargetCaches[i].mTargetPercent;
	}
}

// This = operator does everythinig BUT transfer paramblock references
void morphChannel::operator=(const morphChannel& from)
{
	// Don't allow self->self assignment
	if(&from == this) return;

	mNumPoints = from.mNumPoints;

	mPoints = from.mPoints;
	mDeltas = from.mDeltas;
	mWeights = from.mWeights;

	mTargetCaches = from.mTargetCaches;
	mSel = from.mSel;

	mActive = from.mActive;
	mActiveOverride = from.mActiveOverride;
	mConnection = from.mConnection;
	mCurvature = from.mCurvature;
	mInvalid = from.mInvalid;
	mModded = from.mModded;
	mName = from.mName;
	mNonLocalizedName = from.mNonLocalizedName;
	mNumProgressiveTargs = from.mNumProgressiveTargs;
	mUseLimit = from.mUseLimit;
	mUseSel = from.mUseSel;
	mSpinmin = from.mSpinmin;
	mSpinmax = from.mSpinmax;
	
	mp = from.mp;
	mTargetPercent = from.mTargetPercent;
	iTargetListSelection = from.iTargetListSelection;
}

float morphChannel::GetTargetPercent(const int which) const
{
	if (which < -1 || which >= mNumProgressiveTargs)
		return 0.0f;
	if (which == -1)
		return mTargetPercent;

	return mTargetCaches[which].mTargetPercent;
}

void morphChannel::ReNormalize()
{
	mTargetPercent= 100.0f/(float)(1+mNumProgressiveTargs);
	std::vector<TargetCache>& target_caches = mTargetCaches;
	for (int i = 0; i < mNumProgressiveTargs; i++)
	{
		target_caches[i].mTargetPercent = (2 + i) * mTargetPercent;
	}
}

void morphChannel::SetUpNewController()
{
	int in, out;
	GetBezierDefaultTangentType(in, out);
	SetBezierDefaultTangentType(BEZKEY_STEP, BEZKEY_STEP);
	Control *c = (Control*)CreateInstance(CTRL_FLOAT_CLASS_ID,GetDefaultController(CTRL_FLOAT_CLASS_ID)->ClassID());
	cblock->SetValue(0,0,0.0f);
	cblock->SetController(0,c);
	SetBezierDefaultTangentType(in, out);
}

// Reconstruct the optimization malarky using the current channel's point info
void morphChannel::rebuildChannel()
{
	Point3 tVert;
	int tPc = mNumPoints;
	if (tPc != mp->MC_Local.Count || !mp->MC_Local.CacheValid)
	{
		return;
	}

	mInvalid = FALSE;

	std::vector<Point3>& deltas = mDeltas;
	tbb::parallel_for(
			tbb::blocked_range<int>(0, tPc),
			[this, &deltas](const tbb::blocked_range<int>& range) {
				for (int i = range.begin(); i < range.end(); ++i)
				{
					Point3 DeltaP, tVert = mPoints[i];
					// calculate the delta cache
					DeltaP.x = (tVert.x - mp->MC_Local.points_x[i]) / 100.0f;
					DeltaP.y = (tVert.y - mp->MC_Local.points_y[i]) / 100.0f;
					DeltaP.z = (tVert.z - mp->MC_Local.points_z[i]) / 100.0f;
					deltas[i] = DeltaP;
				}
			},
			tbb::simple_partitioner{});
}

// Generate all the optimzation and geometry data
void morphChannel::buildFromNode( INode *node , BOOL resetTime, TimeValue t, BOOL picked, BOOL inModifiy )
{
	// fix for defect 1346773 - Crash deleting progressive morph target
	if (NULL == node ) {
		return;
	}

	if(resetTime) t = GetCOREInterface()->GetTime();

	ObjectState os = node->EvalWorldState(t);
	mp->tmpValid &= os.obj->ObjectValidity(t); // checkValidity

	int tPc = os.obj->NumPoints();
	Point3 DeltP;
	Point3 tVert;

	if (tPc != mp->MC_Local.Count)
	{
		mNumPoints = 0;
		mActive = FALSE;
		mInvalid = TRUE;
		return;
	}

	if (!mp->MC_Local.CacheValid)
	{
		tPc = 0;
		return;
	}

	mInvalid = FALSE;

	// if the channel hasn't been edited yet, change the 'empty'
	// name to that of the chosen object.
	if (!mModded || picked)
	{
		mName = node->GetName();
		mNonLocalizedName = node->GetName();
	}

	// Set the data into the morphChannel
	mActive = TRUE;
	mModded = TRUE;
	
	// Prepare the channel
	AllocBuffers(tPc, tPc);
	mSel.SetSize(tPc);
	mSel.ClearAll();

	// use Tbb
	mNumPoints = tPc;
	tbb::parallel_for(
			tbb::blocked_range<int>(0, tPc, 5000),
			[this, &os](const tbb::blocked_range<int>& range) {
				for (int i = range.begin(); i < range.end(); ++i)
				{
					Point3 tVert;
					Point3 DeltP;
					tVert = os.obj->GetPoint(i);

					// calculate the delta cache
					DeltP.x = (tVert.x - mp->MC_Local.points_x[i]) / 100.0f;
					DeltP.y = (tVert.y - mp->MC_Local.points_y[i]) / 100.0f;
					DeltP.z = (tVert.z - mp->MC_Local.points_z[i]) / 100.0f;
					mDeltas[i] = DeltP;

					mWeights[i] = os.obj->GetWeight(i);
					mSel.Set(i, os.obj->IsPointSelected(i) ? 1 : 0);

					mPoints[i] = tVert;
				}
			},
			tbb::simple_partitioner{});

	// Update *everything*
	if (inModifiy == FALSE)
	{
		mp->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		mp->NotifyDependents(FOREVER, PART_ALL, REFMSG_SUBANIM_STRUCTURE_CHANGED);
		mp->Update_channelFULL();
		mp->Update_channelParams();
	}
}

const std::vector<Point3>& morphChannel::Points() const
{
	return mPoints;
}
std::vector<Point3>& morphChannel::Points()
{
	return mPoints;
}

const std::vector<Point3>& morphChannel::Deltas() const
{
	return mDeltas;
}
std::vector<Point3>& morphChannel::Deltas()
{
	return mDeltas;
}

const std::vector<double>& morphChannel::Weights() const
{
	return mWeights;
}
std::vector<double>& morphChannel::Weights()
{
	return mWeights;
}

const std::vector<TargetCache>& morphChannel::TargetCaches() const
{
	return mTargetCaches;
}
std::vector<TargetCache>& morphChannel::TargetCaches()
{
	return mTargetCaches;
}

//
// from interface IMorpherChannel interface:
//

bool morphChannel::IsActive() const 
{
	return (TRUE == mActive);
}

const TCHAR* morphChannel::GetName(bool localized) 
{
	return localized ? mName.data() : mNonLocalizedName.data();
}

void morphChannel::SetName(const TCHAR* pName, bool localized) 
{
	if (localized)
	{
		mName = pName;
	}
	else
	{
		mNonLocalizedName = pName;
	}
}

int morphChannel::NumPoints() const 
{
	return mNumPoints;
}

INode* morphChannel::GetTarget(int pTargetIndex) 
{
	INode* morphTarget = nullptr;
	bool valid = (0 <= pTargetIndex && pTargetIndex < (NumProgressiveTargets() + 1));
	if (!valid)
	{
		return nullptr;
	}

	if (0 == pTargetIndex)
	{
		morphTarget = mConnection;
	}
	else
	{
		morphTarget = TargetCaches()[(pTargetIndex - 1)].mTargetINode;
	}

	return morphTarget;
}

int morphChannel::GetTargetCount() const 
{
	int numProgMorphTargets = 0;
	if (NumPoints() > 0)
	{
		// The first target that is directly connected with the morphChannel
		// is not taken as a progressive target.
		// So the total target count should be mNumProgressiveTargets + 1.
		numProgMorphTargets = mNumProgressiveTargs + 1;
	}
	return numProgMorphTargets;
}

Point3 morphChannel::GetTargetPoint(int pTargetIndex, int ptId) 
{
	Point3 point(0, 0, 0);
	DbgAssert((ptId >= 0 && ptId < NumPoints()));
	if (ptId < 0 || ptId >= NumPoints())
		return point;

	if (0 == pTargetIndex)
	{
		point = Points()[ptId];
	}
	else if (pTargetIndex >= 1)
	{
		point = TargetCaches()[(pTargetIndex - 1)].GetPoint(ptId);
	}
	return point;
}

bool morphChannel::SetTargetPercent(int morphIndex, double newValue)
{
	int channelId = mp->GetChannelId(this);
	if (channelId < 0)
		return false;

	if (theHold.Holding())
	{
		theHold.Put(new Restore_FullChannel(mp, channelId));
	}

	if (newValue < MorphR3::kProgressiveTargetWeigthMin)
		newValue = MorphR3::kProgressiveTargetWeigthMin;
	if (newValue > MorphR3::kProgressiveTargetWeigthMax)
		newValue = MorphR3::kProgressiveTargetWeigthMax;
	
	if (0 == morphIndex)
	{
		mTargetPercent = newValue;
	}
	else if (morphIndex >= 1)
	{
		TargetCaches()[morphIndex - 1].mTargetPercent = newValue;
	}
	
	if (morphIndex >= 0 && morphIndex < (NumProgressiveTargets() + 1))
	{
	
	}

#pragma warning(push)
#pragma warning(disable : 4996)
	IMorphClass morphClass;
	morphClass.SortProgressiveTarget(mp, channelId, morphIndex);
#pragma warning(pop)
	return true;
}

double morphChannel::GetTargetPercent(int targetIndex)
{
	bool test = (targetIndex >= 0 && targetIndex <= mNumProgressiveTargs);
	DbgAssert(test);
	if (!test)
	{
		return 0.0;
	}

	float weight = 0.0f;
	if (targetIndex == 0)
	{
		weight = mTargetPercent;
	}
	else if (targetIndex >= 1)
	{
		weight = TargetCaches()[targetIndex - 1].mTargetPercent;
	}

	return weight;
}

Point3 morphChannel::GetPoint(int pId) 
{
	DbgAssert(pId >= 0 && pId < (int)Points().size());
	if (pId >= 0 && pId < (int)Points().size())
	{
		return Points()[pId];
	}
	return Point3();
}

void morphChannel::SetPoint(int pId, Point3& pPoint)
{
	DbgAssert(pId >= 0 && pId < (int)Points().size());
	if (pId >= 0 && pId < (int)Points().size())
	{
		Points()[pId] = pPoint;
	}
}

Point3 morphChannel::GetDelta(int pId) 
{
	if (pId < 0 || pId >= NumPoints())
		return Point3(0, 0, 0);

	return Deltas()[pId];
}

void morphChannel::SetDelta(int pId, Point3& pPoint)
{
	DbgAssert(pId >= 0 || pId < NumPoints());
	if (pId < 0 || pId >= NumPoints())
		return;

	Deltas()[pId] = pPoint;
}

double morphChannel::GetInitPercent() 
{
	float result = 0.0f;
	Interval validity(FOREVER);
	cblock->GetValue(0, 0, result, validity);
	return result;
}

double morphChannel::GetWeight(int pId) 
{
	DbgAssert(pId >= 0 && pId < mNumPoints);
	if (pId >= 0 && pId < mNumPoints)
	{
		return Weights()[pId];
	}

	return 0.0;
}

void morphChannel::SetWeight(int pId, double pValue)
{
	DbgAssert(pId >= 0 && pId < mNumPoints);
	if (pId >= 0 && pId < mNumPoints)
	{
		Weights()[pId] = pValue;
	}
}

INode* morphChannel::GetConnection()
{
	return mConnection;
}

void morphChannel::SetConnection(INode* pNode) 
{
	if (GetMorphNode::IsValidMorphTargetType(mp, pNode, TimeValue(0)))
	{
		ResetMe();
		int chId = mp->GetChannelId(this);
		if (chId < 0)
			return;

		if (theHold.Holding())
		{
			theHold.Put(new Restore_FullChannel(mp, chId));
		}

		// Set a new target for the channel
		int refIDOffset = mp->GetRefIDOffset(chId);
		mp->ReplaceReference(101 + (chId % 100) + refIDOffset, pNode);
		buildFromNode(pNode, FALSE, TimeValue(0), TRUE); // updates UI and dependents
		mNumProgressiveTargs = 0;
		ReNormalize();
	}
}

Control* morphChannel::GetControl()
{
	return cblock->GetController(0);
}

bool morphChannel::IsProgressive() const
{
	return (mNumProgressiveTargs > 0);
}

int morphChannel::NumTargets() const
{
	return mNumProgressiveTargs;
}

bool morphChannel::GetBaseDeltas(Tab<Point3>& deltas) const
{
	int size = static_cast<int>(mPoints.size());
	deltas.SetCount(size);
	Point3* pts = deltas.Addr(0);
	for (int i = 0; i < size; ++i)
	{
		pts[i].x = 100.0 * mDeltas[i].x;
		pts[i].y = 100.0 * mDeltas[i].y;
		pts[i].z = 100.0 * mDeltas[i].z;
	}

	return true;
}

// need to add a MAXScript function to test all cases
bool morphChannel::GetProgressiveTargetDeltas(int pTargetIndex, Tab<Point3>& deltas) const
{
	DbgAssert(pTargetIndex >= 0 || pTargetIndex < mNumProgressiveTargs);
	if (pTargetIndex < 0 || pTargetIndex >= mNumProgressiveTargs)
	{
		return false;
	}

	int size = static_cast<int>(mPoints.size());
	deltas.SetCount(size);
	Point3* pts = deltas.Addr(0);
	const std::vector<TargetCache>& targetCaches = TargetCaches();
	if (pTargetIndex >= 0 && (size_t)pTargetIndex < targetCaches.size())
	{
		const TargetCache& targetCache = targetCaches[pTargetIndex];
		for (int i = 0; i < size; ++i)
		{
			const Point3& ptref(targetCache.mTargetPoints[i]);
			pts[i].x = ptref.x - mp->MC_Local.points_x[i];
			pts[i].y = ptref.y - mp->MC_Local.points_y[i];
			pts[i].z = ptref.z - mp->MC_Local.points_z[i];
		}
	}

	return true;	
}

bool morphChannel::GetAnimationKeysData(Tab<long>& vKeyTime, Tab<float>& vKeyValue)
{
	Control* pControl = GetControl();
	if (pControl)
	{		
		int j, numKeys = pControl->NumKeys();
		vKeyTime.SetCount(numKeys);
		vKeyValue.SetCount(numKeys);

		// assuming BezierFloat (HYBRIDINTERP_FLOAT_CLASS_ID) and scale == 1.0f
		float keyVal, scale = 1.0f;
		Interval valid;
		float* v = vKeyValue.Addr(0);
		long* t = vKeyTime.Addr(0);
		for (j = 0; j < numKeys; ++j)
		{
			TimeValue kt = pControl->GetKeyTime(j);
			long keyTime = kt / GetTicksPerFrame();
			pControl->GetValue(kt, &keyVal, valid);
			keyVal *= scale;
			t[j] = keyTime;
			v[j] = keyVal;
		}
		return true;
	}
	return false;
}

void morphChannel::Reset(bool active, bool modded, int numPoints)
{
	int idx{ -1 };
	if (mp)
	{
		for (int i = 0; i < mp->chanBank.size(); ++i)
		{
			if (this == &mp->chanBank[i])
				idx = i;
			break;
		}
	}
	if (idx >= 0)
	{
		mp->DeleteChannel(idx);
		mActive = active;
		mModded = modded;
		mNumPoints = numPoints;
		Points().resize(numPoints);
		Deltas().resize(numPoints);
		Weights().resize(numPoints);
	}
}