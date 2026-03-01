/*===========================================================================*\
 |
 |  FILE:	wM3_core.cpp
 |			Weighted Morpher for MAX R3
 |			ModifyObject
 |
 |  AUTH:   Harry Denholm
 |			Copyright(c) Kinetix 1999
 |			All Rights Reserved.
 |
 |  HIST:	Started 27-8-98
 |
\*===========================================================================*/

#include "wM3.h"

#include <tbb/parallel_for.h>
#include <tbb/task_group.h>

#include <cassert>
#include <array>

namespace {
float epsilon = 1E-6f;

inline void Bez3D(Point3& b, const Point3* p, const float& u)
{
	float t01[3], t12[3], t02[3], t13[3];

	t01[0] = p[0][0] + (p[1][0] - p[0][0]) * u;
	t01[1] = p[0][1] + (p[1][1] - p[0][1]) * u;
	t01[2] = p[0][2] + (p[1][2] - p[0][2]) * u;

	t12[0] = p[1][0] + (p[2][0] - p[1][0]) * u;
	t12[1] = p[1][1] + (p[2][1] - p[1][1]) * u;
	t12[2] = p[1][2] + (p[2][2] - p[1][2]) * u;

	t02[0] = t01[0] + (t12[0] - t01[0]) * u;
	t02[1] = t01[1] + (t12[1] - t01[1]) * u;
	t02[2] = t01[2] + (t12[2] - t01[2]) * u;

	t01[0] = p[2][0] + (p[3][0] - p[2][0]) * u;
	t01[1] = p[2][1] + (p[3][1] - p[2][1]) * u;
	t01[2] = p[2][2] + (p[3][2] - p[2][2]) * u;

	t13[0] = t12[0] + (t01[0] - t12[0]) * u;
	t13[1] = t12[1] + (t01[1] - t12[1]) * u;
	t13[2] = t12[2] + (t01[2] - t12[2]) * u;

	b[0] = t02[0] + (t13[0] - t02[0]) * u;
	b[1] = t02[1] + (t13[1] - t02[1]) * u;
	b[2] = t02[2] + (t13[2] - t02[2]) * u;
}

struct progressive_data
{
	std::array<Point3, 4> splinepoints;
	int segment = 1;
	float progression = 0.f;
};

progressive_data compute_progressive_data(
		const MorphR3& morpher, const morphChannel& chan, int pointnum, double percent)
{
	progressive_data ret;
	std::array<Point3, 4>& splinepoint = ret.splinepoints;

	// worker variables for progressive modification
	Point3 endpoint[4];
	Point3 temppoint[2];
	Point3 progession;

	float totaltargs = float(chan.mNumProgressiveTargs + 1);
	ret.progression = percent;

	if (ret.progression < 0)
	{
		ret.progression = 0;
	}
	else if (ret.progression > 100)
	{
		ret.progression = 100;
	}

	while (ret.segment <= totaltargs && ret.progression >= chan.GetTargetPercent(ret.segment - 2))
	{
		++ret.segment;
	}

	// figure out which segment we are in
	// on the target is the next segment
	// first point (object) MC_Local.oPoints
	// second point (first target) is chan.mPoints
	// each additional target is targetcache starting from 0
	if (ret.segment == 1)
	{
		endpoint[0] = {
			morpher.MC_Local.points_x[pointnum],
			morpher.MC_Local.points_y[pointnum],
			morpher.MC_Local.points_z[pointnum],
		};
		endpoint[1] = chan.Points()[pointnum];
		endpoint[2] = chan.TargetCaches()[0].GetPoint(pointnum);
	}
	else if (ret.segment == totaltargs)
	{
		int targnum = int(totaltargs - 1);
		for (int j = 2; j >= 0; j--)
		{
			targnum--;
			if (targnum == -2)
			{
				endpoint[0] = {
					morpher.MC_Local.points_x[pointnum],
					morpher.MC_Local.points_y[pointnum],
					morpher.MC_Local.points_z[pointnum],
				};
			}
			else if (targnum == -1)
			{
				temppoint[0] = chan.Points()[pointnum];
			}
			else
			{
				temppoint[0] = chan.TargetCaches()[targnum].GetPoint(pointnum);
			}
			endpoint[j] = temppoint[0];
		}
	}
	else
	{
		int targnum = ret.segment;
		for (int j = 3; j >= 0; j--)
		{
			targnum--;
			if (targnum == -2)
			{
				endpoint[0] = {
					morpher.MC_Local.points_x[pointnum],
					morpher.MC_Local.points_y[pointnum],
					morpher.MC_Local.points_z[pointnum],
				};
			}
			else if (targnum == -1)
			{
				temppoint[0] = chan.Points()[pointnum];
			}
			else
			{
				temppoint[0] = chan.TargetCaches()[targnum].GetPoint(pointnum);
			}
			endpoint[j] = temppoint[0];
		}
	}

	// set the middle knot vectors
	if (ret.segment == 1)
	{
		splinepoint[0] = endpoint[0];
		splinepoint[3] = endpoint[1];
		temppoint[1] = endpoint[2] - endpoint[0];
		temppoint[0] = endpoint[1] - endpoint[0];
		float length = LengthSquared(temppoint[1]);
		if (std::fabs(length) < epsilon)
		{
			splinepoint[1] = endpoint[0];
			splinepoint[2] = endpoint[1];
		}
		else
		{
			splinepoint[2] =
					endpoint[1] - (DotProd(temppoint[0], temppoint[1]) * chan.mCurvature / length) * temppoint[1];
			splinepoint[1] = endpoint[0] + chan.mCurvature * (splinepoint[2] - endpoint[0]);
		}
	}
	else if (ret.segment == totaltargs)
	{
		splinepoint[0] = endpoint[1];
		splinepoint[3] = endpoint[2];
		temppoint[1] = endpoint[2] - endpoint[0];
		temppoint[0] = endpoint[1] - endpoint[2];
		float length = LengthSquared(temppoint[1]);
		if (std::fabs(length) < epsilon)
		{
			splinepoint[1] = endpoint[0];
			splinepoint[2] = endpoint[1];
		}
		else
		{
			Point3 p = (DotProd(temppoint[1], temppoint[0]) * chan.mCurvature / length) * temppoint[1];
			splinepoint[1] =
					endpoint[1] - (DotProd(temppoint[1], temppoint[0]) * chan.mCurvature / length) * temppoint[1];
			splinepoint[2] = endpoint[2] + chan.mCurvature * (splinepoint[1] - endpoint[2]);
		}
	}
	else
	{
		temppoint[1] = endpoint[2] - endpoint[0];
		temppoint[0] = endpoint[1] - endpoint[0];
		float length = LengthSquared(temppoint[1]);
		splinepoint[0] = endpoint[1];
		splinepoint[3] = endpoint[2];
		if (std::fabs(length) < epsilon)
		{
			splinepoint[1] = endpoint[0];
		}
		else
		{
			splinepoint[1] =
					endpoint[1] + (DotProd(temppoint[0], temppoint[1]) * chan.mCurvature / length) * temppoint[1];
		}

		temppoint[1] = endpoint[3] - endpoint[1];
		temppoint[0] = endpoint[2] - endpoint[1];
		length = LengthSquared(temppoint[1]);
		if (std::fabs(length) < epsilon)
		{
			splinepoint[2] = endpoint[1];
		}
		else
		{
			splinepoint[2] =
					endpoint[2] - (DotProd(temppoint[0], temppoint[1]) * chan.mCurvature / length) * temppoint[1];
		}
	}

	return ret;
}

// This here function is used for performance.
// Basically, a lot of the if statements are converted to compile-time,
// to help the cpu prediciton.
template <bool FilterSel, bool HasSubSel, bool DoProgressive>
void morph_channel_impl(const morphChannel& chan, double percent, MorphR3& morpher)
{
	const std::vector<Point3>& chan_deltas = chan.Deltas();
	const std::vector<double>& chan_weights = chan.Weights();

	tbb::parallel_for(tbb::blocked_range<int>{ 0, chan.mNumPoints }, [&](const tbb::blocked_range<int>& range) {
		for (int pointnum = range.begin(); pointnum < range.end(); ++pointnum)
		{
			const bool chSelState = (pointnum >= 0 && pointnum < chan.mSel.GetSize()) ? chan.mSel[pointnum] : false;
			const bool state = !(FilterSel || chSelState);
			if (state) 
			{
				continue;
			}

			{
				// calculate the differences
				// decay by the weighted vertex amount, to support soft selection
				double cache_weight = morpher.MC_Local.weights[pointnum];
				double deltW = (chan_weights[pointnum] - cache_weight) * percent;
				morpher.weights_cache[pointnum] += deltW;
			}

			// Get softselection, if applicable
			double decay = 1.0f;
			if constexpr (HasSubSel)
			{
				decay = morpher.MC_Local.selection_weights[pointnum];
			}

			// some worker variables
			if constexpr (!DoProgressive)
			{
				float delta_x = float(chan_deltas[pointnum].x * percent * decay);
				float delta_y = float(chan_deltas[pointnum].y * percent * decay);
				float delta_z = float(chan_deltas[pointnum].z * percent * decay);
				morpher.delta_x_cache[pointnum] += delta_x;
				morpher.delta_y_cache[pointnum] += delta_y;
				morpher.delta_z_cache[pointnum] += delta_z;
			}
			else
			{
				progressive_data data = compute_progressive_data(morpher, chan, pointnum, percent);

				// this is the normalizing equation
				float targetpercent1 = chan.GetTargetPercent(data.segment - 3);
				float targetpercent2 = chan.GetTargetPercent(data.segment - 2);

				float top = data.progression - targetpercent1;
				float bottom = targetpercent2 - targetpercent1;
				float u = top / bottom;

				// this is just the bezier calculation
				Point3 progession;
				Bez3D(progession, data.splinepoints.data(), u);
				float delta_x = float((progession[0] - morpher.MC_Local.points_x[pointnum]) * decay);
				float delta_y = float((progession[1] - morpher.MC_Local.points_y[pointnum]) * decay);
				float delta_z = float((progession[2] - morpher.MC_Local.points_z[pointnum]) * decay);
				morpher.delta_x_cache[pointnum] += delta_x;
				morpher.delta_y_cache[pointnum] += delta_y;
				morpher.delta_z_cache[pointnum] += delta_z;
			}
		} // nmPts cycle
	});
}

void morph_channel(const morphChannel& chan, int os_subselstate, bool filter_selection, double percent, MorphR3& morpher)
{
	// Remove the if statements from the runtime computation.
	if (filter_selection)
	{
		if (os_subselstate == 0)
		{
			if (chan.mNumProgressiveTargs == 0)
			{
				morph_channel_impl<true, false, false>(chan, percent, morpher);
			}
			else
			{
				morph_channel_impl<true, false, true>(chan, percent, morpher);
			}
		}
		else
		{
			if (chan.mNumProgressiveTargs == 0)
			{
				morph_channel_impl<true, true, false>(chan, percent, morpher);
			}
			else
			{
				morph_channel_impl<true, true, true>(chan, percent, morpher);
			}
		}
	}
	else
	{
		if (os_subselstate == 0)
		{
			if (chan.mNumProgressiveTargs == 0)
			{
				morph_channel_impl<false, false, false>(chan, percent, morpher);
			}
			else
			{
				morph_channel_impl<false, false, true>(chan, percent, morpher);
			}
		}
		else
		{
			if (chan.mNumProgressiveTargs == 0)
			{
				morph_channel_impl<false, true, false>(chan, percent, morpher);
			}
			else
			{
				morph_channel_impl<false, true, true>(chan, percent, morpher);
			}
		}
	}
}
} // namespace

void MorphR3::ModifyObject(TimeValue t, ModContext& mc, ObjectState* os, INode* node)
{
	Update_channelValues();

	// This will see if the local cached object is valid and update it if not
	// It will now also call a full channel rebuild to make sure their deltas are
	// accurate to the new cached object
	if (!MC_Local.AreWeCached())
	{
		UI_MAKEBUSY

		MC_Local.MakeCache(os->obj);
		mSelectionCacheTime = t;

		for (int i = 0; i < chanBank.size(); i++)
		{
			if (chanBank[i].mActive)
			{
				chanBank[i].rebuildChannel();
			}
		}

		UI_MAKEFREE
	}
	//if we did not rebuild the full cache, we might just need to rebuild the 
	//selection cache.  This is needed since selections 
	//can be animated below the morpher
	else 
	{
		//check if the selection cache time is valid
		Interval selInterval = os->obj->ChannelValidity(t, SELECT_CHAN_NUM);
		if (!selInterval.InInterval(mSelectionCacheTime))
		{
			//if not rebuild the selection cache
			MC_Local.MakeSelectionCache(os->obj);
			mSelectionCacheTime = t;
		}

	}

	Interval valid = FOREVER;

	// AUTOLOAD
	int itmp;
	pblock->GetValue(PB_CL_AUTOLOAD, 0, itmp, valid);

	if (itmp == 1)
	{
		tmpValid = valid;
		for (int k = 0; k < chanBank.size(); k++)
		{
			if (chanBank[k].mConnection)
			{
				chanBank[k].buildFromNode(chanBank[k].mConnection, FALSE, t, FALSE, TRUE);
				for (int i = 0; i < chanBank[k].mNumProgressiveTargs; i++)
				{
					TargetCache& tcache = chanBank[k].TargetCaches()[i];
					tcache.Init(tcache.mTargetINode);
				}
			}
		}
		valid = tmpValid;
	}

	// Get count from host
	int hmCount = os->obj->NumPoints();

	int glUsesel;
	pblock->GetValue(PB_OV_USESEL, t, glUsesel, valid);

	BOOL glUseLimit;
	float glMAX, glMIN;
	pblock->GetValue(PB_OV_USELIMITS, t, glUseLimit, valid);
	pblock->GetValue(PB_OV_SPINMAX, t, glMAX, valid);
	pblock->GetValue(PB_OV_SPINMIN, t, glMIN, valid);

	// Calling GetSubselState repeatedly is actually costly.
	const DWORD os_subselstate = os->obj->GetSubselState();

	// Contains valid channel index and its GetValue float (percentage).
	std::vector<std::pair<size_t, float>> channel_percents;
	channel_percents.reserve(chanBank.size());

	// Can't thread this because of paramblock GetValue.
	for (size_t i = 0; i < chanBank.size(); ++i)
	{
		morphChannel& channel(chanBank[i]);

		if (!channel.mActive)
		{
			continue;
		}

		// temp fix for diff. pt counts
		if (channel.mNumPoints != hmCount)
		{
			channel.mInvalid = TRUE;
			continue;
		}

		// This channel is considered okay to use
		channel.mInvalid = FALSE;

		// Is this channel flagged as inactive?
		if (channel.mActiveOverride == FALSE)
		{
			continue;
		}

		// get morph percentage for this channel
		float percent;
		channel.cblock->GetValue(0, t, percent, valid);

		if (percent == 0.f)
		{
			continue;
		}

		// Clamp the channel values to the limits
		if (channel.mUseLimit || glUseLimit)
		{
			int Pmax = int(channel.mSpinmax);
			int Pmin = int(channel.mSpinmin);

			if (glUseLimit)
			{
				Pmax = int(glMAX);
				Pmin = int(glMIN);
			}
			percent = std::clamp<float>(percent, Pmin, Pmax);
		}

		if (percent == 0.f)
		{
			continue;
		}

		// Store channel index and percent for further processing.
		channel_percents.push_back({ i, percent });
	}


	// These are our morph deltas / point
	// They get built by cycling through the points and generating
	// the difference data, summing it into these tables and then
	// appling the changes at the end.
	// This will leave us with the total differences per point on the
	// local mesh. We can then rip through and apply them quickly

	// First, initialize the data.
	tbb::task_group g;
	g.run([hmCount, this]() {
		delta_x_cache.resize(hmCount);
		std::copy(MC_Local.points_x.begin(), MC_Local.points_x.end(), delta_x_cache.begin());
	});
	g.run([hmCount, this]() {
		delta_y_cache.resize(hmCount);
		std::copy(MC_Local.points_y.begin(), MC_Local.points_y.end(), delta_y_cache.begin());
	});
	g.run([hmCount, this]() {
		delta_z_cache.resize(hmCount);
		std::copy(MC_Local.points_z.begin(), MC_Local.points_z.end(), delta_z_cache.begin());
	});
	g.run_and_wait([hmCount, this]() {
		weights_cache.resize(hmCount);
		std::copy(MC_Local.weights.begin(), MC_Local.weights.end(), weights_cache.begin());
	});

	assert(delta_x_cache.size() == delta_y_cache.size());
	assert(delta_x_cache.size() == delta_z_cache.size());
	assert(delta_x_cache.size() == weights_cache.size());

	// --------------------------------------------------- MORPHY BITS
	// cycle through channels, process the channels that need morphy bits.
	for (const std::pair<size_t, float>& p : channel_percents)
	{
		size_t idx = p.first;
		if (idx == (std::numeric_limits<size_t>::max)())
		{
			continue;
		}

		float percent = p.second;
		const morphChannel& channel = chanBank[idx];
		bool filter_selection = !channel.mUseSel && (glUsesel == 0);

		morph_channel(channel, os_subselstate, filter_selection, double(percent), *this);
	}

	// TH 10/12/15: MAXX-26476 -- Multithreading is crashing; turning it "off" for the time being by setting to 1 thread
	// "Real issue" is that ShapeObject::InvalidateGeomCache is not thread-safe and cached mesh freeing is causing a
	// crash. Created Jira defect MAXX-26886 to revisit & fix for Kirin
	for (int k = 0; k < hmCount; ++k)
	{
		if (os_subselstate != 0 && !MC_Local.sel[k])
		{
			continue;
		}

		Point3 fVert(delta_x_cache[k], delta_y_cache[k], delta_z_cache[k]);
		os->obj->SetPoint(k, fVert);
		os->obj->SetWeight(k, weights_cache[k]);
	}

	// Captain Hack Returns...
	// Support for saving of modifications to a channel
	// Most of this is just duped from buildFromNode (delta/point/calc)
	if (recordModifications)
	{
		int tChan = recordTarget;
		morphChannel& morChannel = chanBank[tChan];
		int tPc = hmCount;

		// Prepare the channel
		morChannel.AllocBuffers(tPc, tPc);
		morChannel.mNumPoints = 0;

		std::vector<Point3>& points = morChannel.Points();
		std::vector<Point3>& deltas = morChannel.Deltas();
		std::vector<double>& weights = morChannel.Weights();
		for (int x = 0; x < tPc; ++x)
		{
			Point3 tVert;
			tVert.x = delta_x_cache[x];
			tVert.y = delta_y_cache[x];
			tVert.z = delta_z_cache[x];

			double wtmp = weights_cache[x];

			// calculate the delta cache
			Point3 DeltP;
			DeltP.x = (tVert.x - MC_Local.points_x[x]) / 100.0f;
			DeltP.y = (tVert.y - MC_Local.points_y[x]) / 100.0f;
			DeltP.z = (tVert.z - MC_Local.points_z[x]) / 100.0f;

			points[x] = tVert;
			deltas[x] = DeltP;
			weights[x] = wtmp;
		}
		morChannel.mNumPoints = tPc;

		recordModifications = FALSE;
		recordTarget = 0;
		morChannel.mInvalid = FALSE;
	}
	// End of record

	if (itmp == 1)
		valid = Interval(t, t);

	os->obj->UpdateValidity(GEOM_CHAN_NUM, valid);
	os->obj->PointsWereChanged();
}

void MorphR3::DisplayMemoryUsage(void)
{
	if (!hwAdvanced)
		return;
	float tmSize = sizeof(*this);
	for (int i = 0; i < chanBank.size(); i++)
		tmSize += chanBank[i].getMemSize();
	TCHAR s[20] = {};
	_stprintf_s(s, _T("%i KB"), (int)tmSize / 1000);
	SetWindowText(GetDlgItem(hwAdvanced, IDC_MEMSIZE), s);
}


BOOL IMorphClass::AddProgessiveMorph(MorphR3* mp, int morphIndex, INode* node)
{
	Interval valid;

	if (NULL == mp || mp->ip == NULL || NULL == node)
		return FALSE;

	ObjectState os = node->GetObjectRef()->Eval(mp->ip->GetTime());

	if (os.obj->IsDeformable() == FALSE)
		return FALSE;

	// Check for same-num-of-verts-count
	if (os.obj->NumPoints() != mp->MC_Local.Count)
		return FALSE;

	node->BeginDependencyTest();
	mp->NotifyDependents(FOREVER, 0, REFMSG_TEST_DEPENDENCY);
	if (node->EndDependencyTest())
	{
		return FALSE;
	}

	// check to make sure that the max number of progressive targets will not be exceeded
	//
	morphChannel& bank = mp->chanBank[morphIndex];
	if (NULL == bank.mConnection || bank.mNumProgressiveTargs >= MAX_PROGRESSIVE_TARGETS)
	{
		return FALSE;
	}


	if (mp->CheckMaterialDependency())
		return FALSE;
	// Make the node reference, and then ask the channel to load itself

	if (theHold.Holding())
		theHold.Put(new Restore_FullChannel(mp, morphIndex));

	int refIDOffset = mp->GetRefIDOffset(morphIndex);
	int refnum = ((morphIndex % 100) * MAX_PROGRESSIVE_TARGETS) + 201 + bank.mNumProgressiveTargs;
	mp->ReplaceReference(refnum + refIDOffset, node);
	bank.InitTargetCache(bank.mNumProgressiveTargs, node);
	bank.mNumProgressiveTargs++;
	assert(bank.mNumProgressiveTargs <= MAX_PROGRESSIVE_TARGETS);
	bank.ReNormalize();
	mp->Update_channelParams();
	return TRUE;
}

BOOL IMorphClass::DeleteProgessiveMorph(MorphR3* mp, int morphIndex, int progressiveMorphIndex)
{

	morphChannel& channel = mp->chanBank[morphIndex];


	if (!&channel || !channel.mNumProgressiveTargs)
		return FALSE;

	int targetnum = progressiveMorphIndex;

	if (theHold.Holding())
		theHold.Put(new Restore_FullChannel(mp, morphIndex));

	std::vector<TargetCache>& target_caches = channel.TargetCaches();
	if (targetnum == 0)
	{
		if (channel.mNumProgressiveTargs)
		{
			channel = channel.TargetCaches()[0];
			for (int i = 0; i < channel.mNumProgressiveTargs - 1; i++)
			{
				target_caches[i] = channel.TargetCaches()[i + 1];
			}
			target_caches[channel.mNumProgressiveTargs - 1].Clear();
		}
	}
	else if (targetnum > 0 && channel.mNumProgressiveTargs && targetnum <= channel.mNumProgressiveTargs)
	{
		for (int i = targetnum; i < channel.mNumProgressiveTargs; i++)
		{
			target_caches[i - 1] = channel.TargetCaches()[i];
		}
		target_caches[channel.mNumProgressiveTargs - 1].Clear();
	}

	// reset the references
	channel.ResetRefs(mp, progressiveMorphIndex);

	channel.mNumProgressiveTargs--;

	channel.ReNormalize();
	channel.iTargetListSelection--;
	if (channel.iTargetListSelection < 0)
	{
		channel.iTargetListSelection = 0;
	}

	channel.rebuildChannel();
	mp->Update_channelFULL();

	if (theHold.Holding())
	{
		theHold.Put(new Restore_Display(mp));
	}

	return TRUE;
}

void IMorphClass::SwapMorphs(MorphR3* mp, const int from, const int to, BOOL swap)
{

	if (theHold.Holding())
	{
		theHold.Put(new Restore_Display(mp));
		theHold.Put(new Restore_FullChannel(mp, from, FALSE));
		theHold.Put(new Restore_FullChannel(mp, to, FALSE));
	}

	if (swap)
		mp->ChannelOp(from, to, OP_SWAP);
	else
		mp->ChannelOp(from, to, OP_MOVE);

	if (theHold.Holding())
	{
		theHold.Put(new Restore_Display(mp));
	}
}


void IMorphClass::SwapPTargets(MorphR3* mp, const int morphIndex, const int from, const int to, const bool isundo)
{
	/////////////////////////////////
	int currentChan = morphIndex;
	morphChannel& cBank = mp->chanBank[currentChan];
	if (from < 0 || to < 0 || from > cBank.NumProgressiveTargets() || to > cBank.NumProgressiveTargets())
		return;


	int refIDOffsetFrom = mp->GetRefIDOffset(to);
	int refIDOffsetTo = mp->GetRefIDOffset(from);

	if (from != 0 && to != 0)
	{
		TargetCache toCache(cBank.TargetCaches()[to - 1]);

		float wa, wb;
		wa = cBank.TargetCaches()[to - 1].mTargetPercent;
		wb = cBank.TargetCaches()[from - 1].mTargetPercent;

		std::vector<TargetCache>& target_caches = cBank.TargetCaches();
		target_caches[to - 1] = cBank.TargetCaches()[from - 1];
		target_caches[from - 1] = toCache;

		target_caches[to - 1].mTargetPercent = wb;
		target_caches[from - 1].mTargetPercent = wa;

		mp->ReplaceReference(
				mp->GetRefNumber(currentChan, from) + refIDOffsetFrom, cBank.TargetCaches()[from - 1].RefNode());
		mp->ReplaceReference(
				mp->GetRefNumber(currentChan, to) + refIDOffsetTo, cBank.TargetCaches()[to - 1].RefNode());
	}
	else // switch channel and first targetcache
	{

		float wa, wb;
		wa = cBank.TargetCaches()[0].mTargetPercent;
		wb = cBank.mTargetPercent;

		TargetCache tempCache(cBank.TargetCaches()[0]);
		cBank.TargetCaches()[0] = cBank;
		cBank = tempCache;

		cBank.TargetCaches()[0].mTargetPercent = wb;
		cBank.mTargetPercent = wa;

		int refIDOffset = mp->GetRefIDOffset(currentChan);
		mp->ReplaceReference(101 + currentChan % 100 + refIDOffset, cBank.mConnection);
		mp->ReplaceReference(mp->GetRefNumber(currentChan, 0) + refIDOffset, cBank.TargetCaches()[0].RefNode());
	}
}


void IMorphClass::SetTension(MorphR3* mp, int morphIndex, float tension)
{
	if (theHold.Holding())
		theHold.Put(new Restore_FullChannel(mp, morphIndex));

	mp->chanBank[morphIndex].mCurvature = tension;

	if (theHold.Holding())
		theHold.Put(new Restore_Display(mp));

	mp->NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
	mp->ip->RedrawViews(mp->ip->GetTime(), REDRAW_INTERACTIVE, NULL);
}

void IMorphClass::HoldMarkers(MorphR3* mp)
{
	if (theHold.Holding())
		theHold.Put(new Restore_Marker(mp));
};

void IMorphClass::HoldChannel(MorphR3* mp, int channel)
{
	if (theHold.Holding())
		theHold.Put(new Restore_FullChannel(mp, channel));
}


BaseInterface* MorphR3::GetInterface(Interface_ID in_interfaceID)
{
	if (in_interfaceID == I_MORPHER_INTERFACE_ID)
	{
		IMorpher* imorph = static_cast<IMorpher*>(this);
		return imorph;
	}		
	else
		return Modifier::GetInterface(in_interfaceID);
}

//
// from IMorpher interface:

void MorphR3::SetNumChannels(int numChannels)
{
	if (numChannels < 0)
		return;
	if (numChannels >= (int)chanBank.size())
	{
		while (numChannels >= (int)chanBank.size())
		{
			Add100Channels();
		}
	}
}

int MorphR3::NumChannels() const
{
	return static_cast<int>(chanBank.size());
}

IMorpherChannel* MorphR3::GetChannel(int pChnlId, bool pDefaultInit /* = false */)
{
	if (pChnlId < 0 || pChnlId >= chanBank.size())
	{
		return nullptr;
	}

	IMorpherChannel* pChnl = static_cast<IMorpherChannel*>(&chanBank[pChnlId]);
	if (pDefaultInit && pChnl)
	{				
		pChnl->Reset(false, false, 0);
	}

	return pChnl;
}

void MorphR3::DeleteAllChannels()
{
	for (int i = NumChannels() - 1; i >= 0; i--)
	{
		DeleteChannel(i);
	}
}

void MorphR3::SetMaxInterface(IObjParam* pMaxInterface)
{
	ip = pMaxInterface;
}

bool MorphR3::MakeCache(Object* pObj)
{
	if (nullptr == pObj)
	{
		return false;
	}

	bool doRebuildCache = false;
	// If the morpher is already applied to an object different than the one
	// passed in as parameter, reject rebuilding the cache
	CountModContextProc modContextEnumProc;
	EnumModContexts(&modContextEnumProc);
	int numModContexts = modContextEnumProc.GetModContextCount();
	if (0 == numModContexts)
	{
		doRebuildCache = true;
	}
	else
	{
		if (1 == numModContexts)
		{
			ModContext* modContext = modContextEnumProc.GetModContext(0);
			DbgAssert(modContext != nullptr);
			IDerivedObject* derivedObj{ nullptr };
			int modIndex = 0;
			// Get the derived object corresponding to the ModContext
			GetIDerivedObject(modContext, derivedObj, modIndex);
			DbgAssert(derivedObj != nullptr);
			if (derivedObj != nullptr)
			{
				Object* baseObject = derivedObj->GetObjRef();
				baseObject = baseObject->FindBaseObject();
				doRebuildCache = (baseObject == pObj);
			}
		}
	}

	if (doRebuildCache)
	{
		NukeCache();
		// Evaluate the object. This effectively updates its cache.
		MC_Local.MakeCache(pObj);
		return (TRUE == MC_Local.AreWeCached());
	}

	return false;
}

void MorphR3::NukeCache()
{
	MC_Local.NukeCache();
}

bool MorphR3::AddProgressiveMorph(int pChannelIndex, INode* node)
{	
	if (pChannelIndex < 0 || pChannelIndex >= (int)chanBank.size())
		return false;

	morphChannel* pChannel = &chanBank[pChannelIndex];
	if (pChannel) 
	{
//#pragma warning(push)
//#pragma warning(disable : 4996)
		Interval valid;

		if (nullptr == ip || nullptr == node)
			return FALSE;

		ObjectState os = node->GetObjectRef()->Eval(ip->GetTime());
		if (os.obj->IsDeformable() == FALSE)
			return false;

		// Check for same-num-of-verts-count
		if (os.obj->NumPoints() != MC_Local.Count)
			return false;

		node->BeginDependencyTest();
		NotifyDependents(FOREVER, 0, REFMSG_TEST_DEPENDENCY);
		if (node->EndDependencyTest())
		{
			return false;
		}


		// check to make sure that the max number of progressive targets will not be exceeded
		//
		if (nullptr == pChannel->GetConnection() || pChannel->NumTargets() >= MAX_PROGRESSIVE_TARGETS)
		{
			return false;
		}

		if (CheckMaterialDependency())
			return false;

		if (theHold.Holding())
			theHold.Put(new Restore_FullChannel(this, pChannelIndex));

		int refIDOffset = GetRefIDOffset(pChannelIndex);
		int refnum = ((pChannelIndex % 100) * MAX_PROGRESSIVE_TARGETS) + 201 + pChannel->NumTargets();
		ReplaceReference(refnum + refIDOffset, node);
		pChannel->InitTargetCache(pChannel->mNumProgressiveTargs, node);
		pChannel->mNumProgressiveTargs++;
		assert(pChannel->mNumProgressiveTargs <= MAX_PROGRESSIVE_TARGETS);
		pChannel->ReNormalize();
		Update_channelParams();
//#pragma warning(pop)
		return true;
	}

	return false;
}

namespace MorpherHelpers {
TriObject* GetTriObjectFromNode(INode* node, int& deleteIt)
{
	deleteIt = FALSE;
	Object* obj = node->EvalWorldState(0).obj;
	if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)))
	{
		TriObject* tri = (TriObject*)obj->ConvertToType(0, Class_ID(TRIOBJ_CLASS_ID, 0));
		if (obj != tri)
			deleteIt = TRUE;
		return tri;
	}
	else
	{
		return nullptr;
	}
}

bool IsMesh(INode* pNode, int pTime)
{
	Object* l3dsGeo = pNode->EvalWorldState(pTime).obj;
	if (l3dsGeo->SuperClassID() == GEOMOBJECT_CLASS_ID)
	{
		if (l3dsGeo->ClassID() == Class_ID(TRIOBJ_CLASS_ID, 0) ||
				l3dsGeo->ClassID() == Class_ID(EDITTRIOBJ_CLASS_ID, 0))
		{
			return true;
		}
	}
	return false;
}

bool GetAnimationKeysData(Control* pControl, std::vector<std::pair<long, float>>& keys)
{
	keys.clear();
	if (!pControl)
	{
		return false;
	}

	int j, numKeys = pControl->NumKeys();

	// for TCB controls we will need the keyframe interface
	// IKeyControl* ikeys = (IKeyControl*)(pMorphCtrl)->GetInterface(I_KEYCONTROL);

	// assuming BezierFloat (HYBRIDINTERP_FLOAT_CLASS_ID) and scale == 1.0f
	float keyVal, scale = 1.0f;
	Interval valid;
	keys.resize(numKeys);
	for (j = 0; j < numKeys; ++j)
	{
		TimeValue kt = pControl->GetKeyTime(j);
		long keytime = kt / GetTicksPerFrame();
		pControl->GetValue(kt, &keyVal, valid);
		keyVal *= scale;
		keys[j] = std::pair<long, float>(keytime, keyVal);
	}
	return true;
}

bool ExportMorpherChannelData(INode* lMaxNode, 
	IMorpherChannel* pChannel, std::vector<std::wstring>& vTargetsNames,
	std::vector<std::pair<long, float>>& vTargetsKeys, 
	std::vector<double>& vTargetsPercents,
	std::vector<std::vector<Point3>>& vTargetsData)
{
	if (!lMaxNode || !pChannel)
	{
		return false;
	}
	TSTR channelName = pChannel->GetName(true);
	double chInitPercent = pChannel->GetInitPercent();
	int i;
	Point3 vert;
	TimeValue cTime = GetCOREInterface()->GetTime();
	// we might need to transform the target's points to the source's space!
	Matrix3 tm = lMaxNode->GetObjectTM(cTime);
	Matrix3 ptm = lMaxNode->GetNodeTM(cTime);
	tm = tm * Inverse(ptm);

	int numVerts = pChannel->NumPoints();
	int numTargets = pChannel->GetTargetCount();
	vTargetsNames.resize(numTargets);
	vTargetsData.resize(numTargets);
	vTargetsPercents.resize(numTargets);
	for (int lTargetIndex = 0; lTargetIndex < numTargets; ++lTargetIndex)
	{
		INode* lTarget = pChannel->GetTarget(lTargetIndex);
		TSTR lTargetName = _T("");
		if (lTarget)
		{
			lTargetName = lTarget->GetName();
			if (lTargetName.Length() == 0)
			{
				lTargetName.printf(L"%s_%d", channelName, lTargetIndex);
			}
		}
		vTargetsNames[lTargetIndex] = lTargetName.data();
		vTargetsPercents[lTargetIndex] = 100.0;
		if (pChannel->GetTargetPercent(lTargetIndex) > 0.0)
		{
			vTargetsPercents[lTargetIndex] = pChannel->GetTargetPercent(lTargetIndex);
		}
		std::vector<Point3>& vData(vTargetsData[lTargetIndex]);
		Point3 vPos;
		// ?????????????????????????????????????????????
		// ???why we are not just focussing on deltas???
		// ?????????????????????????????????????????????
		for (i = 0; i < numVerts; ++i)
		{
			vPos = pChannel->GetTargetPoint(lTargetIndex, i);
			// we might need to transform this point to the source reference system!!
			vData[i] = vPos;
		}

		// export blend shape channel animation
		Control* pMorphCtrl = pChannel->GetControl();
		if (pMorphCtrl)
		{
			GetAnimationKeysData(pMorphCtrl, vTargetsKeys);
		}
	}

	return true;
}

bool ExportMorpherData(INode* pMaxNode)
{
	int lCount;
	int lMaxCount;
	bool succeeded{ true };
	Object* pObj = pMaxNode->GetObjectRef();
	IDerivedObject* pDerObj{ nullptr };

	if (pObj)
	{
		pDerObj = (IDerivedObject*)pObj;
		int nModifiers = pDerObj->NumModifiers();
		int nModifierIndex = 0;
		TimeValue cTime = GetCOREInterface()->GetTime();
		// we need a way to transform a target's vertex position into the source space! do we need it?
		Matrix3 tm = pMaxNode->GetObjectTM(cTime);
		Matrix3 ptm = pMaxNode->GetNodeTM(cTime);
		tm = tm * Inverse(ptm);
		while (nModifierIndex < nModifiers)
		{
			// Try to find the morpher modifier object
			Modifier* lMorpher = pDerObj->GetModifier(nModifierIndex++);
			if (!(lMorpher && lMorpher->SuperClassID() == OSM_CLASS_ID && lMorpher->ClassID() == MR3_CLASS_ID))
			{
				continue;
			}

			IMorpher* iMorpher = (IMorpher*)lMorpher->GetInterface(I_MORPHER_INTERFACE_ID);
			if (iMorpher)
			{
				TSTR lMorpherName = lMorpher->GetObjectName(true);
				DebugPrint(L"Morpher object name: %s\n", lMorpherName.data());
				
				std::vector<XChannelData> data;
				data.reserve(iMorpher->NumChannels());
				// Set the blend shape channels and target shapes for this node
				for (lCount = 0, lMaxCount = iMorpher->NumChannels(); lCount < lMaxCount; lCount++)
				{
					IMorpherChannel* lChannel = iMorpher->GetChannel(lCount);
					if (lChannel && lChannel->IsActive())
					{
						// We assume it is a mesh...
						XChannelData xdata;
						ExportMorpherChannelData(pMaxNode, 
							lChannel,
							xdata.vTargetsNames,
							xdata.vTargetsKeys, 
							xdata.vTargetsPercents, 
							xdata.vTargetsData);
						data.push_back(xdata);
					}

				} // For each channel
			}
		}
	}

	return succeeded;
}

} // namespace MorpherHelpers
