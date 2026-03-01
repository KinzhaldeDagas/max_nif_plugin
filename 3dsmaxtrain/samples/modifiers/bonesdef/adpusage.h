//**************************************************************************/
// Copyright (c) 2022 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#if __has_include("ADPForSamples.h")
	#include "ADPForSamples.h"
#else
#include <chrono>
namespace ADPForSamples {
class ADPSkinEventLogger
{
public:
	static std::chrono::steady_clock::time_point tEnterEditEnvelopeMode;
	static std::chrono::steady_clock::time_point tEnterPaintWeightMode;
	static int sTotalModPoints;

	enum class SkinWaypointType
	{
		SKIN_STATISTICS = 0,
		SKIN_FEATURE_DURATION,
		SKIN_FEATURE_USAGE,
	};

	enum class SkinFeatureUsage_EventType
	{
		SKIN_JOINT_ANGLE_DEFORMER = 0,
		SKIN_MIRROR_MODE,
		SKIN_PAINT_BLEND_WEIGHTS,
		SKIN_REMOVE_ZERO_WEIGHTS,
		SKIN_ZERO_WEIGHTS_VALUE,
		SKIN_SELECT_VERTICES,
		SKIN_WEIGHT_TABLE,
		SKIN_WEIGHT_TOOL,
		TOTAL_FEATURE_USAGE = 8
	};

	enum class SkinStatistics_EventType
	{
		SKIN_POLYGON_COUNT = 0,
		SKIN_VERTEX_COUNT,
		SKIN_OBJECT_COUNT,
		SKIN_BONES_COUNT,
		SKIN_BONEAFFECTS_LIMIT,
		SKIN_POLY_VERTEX_COUNT,
		SKIN_BONE_VERTEX_BONEAFFECTS_LIMIT,
		SKIN_STATISTIC_ALL,
		TOTAL_STATISTIC = 8
	};

	enum class SkinFeatureDuration_EventType
	{
		SKIN_APPLY_LOAD_TIME = 0,
		SKIN_EDIT_ENVELOPES_STATE,
		SKIN_EDIT_ENVELOPES_TIME,
		SKIN_VOXEL_SOLVER_TIME,
		SKIN_PAINT_WEIGHT_STATE,
		SKIN_PAINT_WEIGHT_TIME,
		TOTAL_FEATURE_DURATION = 6
	};

	static void EnterEditEnvelopeMode()
	{
		return;
	}
	static void ExitEditEnvelopeMode()
	{
		return;
	}

	static void EnterPaintWeightMode()
	{
		return;
	}
	static void ExitPaintWeightMode()
	{
		return;
	}

	template <typename T1, typename T2, typename... Args>
	static void LogSkinADPEvent(T1, T2, const Args&... args)
	{
		return;
	}
};
} // namespace ADPForSamples
#endif