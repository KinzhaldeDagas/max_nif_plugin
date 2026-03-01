/**********************************************************************
 *<
	FILE: sctex.cpp

	DESCRIPTION: A ShadeContext for rendering texture maps

	CREATED BY: Rolf Berteig (took code from mtlrend.cpp

	HISTORY: 2/02/96

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#include "mods.h"
#include "sctex.h"
#include <ColorManagement/IColorPipelineMgr.h>

using namespace MaxSDK::ColorManagement;

SCTex::SCTex()
{
	tiling = 1.0f;
	mtlNum = 0;
	doMaps = TRUE;
	filterMaps = TRUE;
	shadow = FALSE;
	backFace = FALSE;
	curTime = 0;
	norm = Point3(0, 0, 1);
	view = Point3(0, 0, -1);
	ResetOutput();
}

Box3 SCTex::ObjectBox()
{
	return Box3(Point3(0, 0, 0), Point3(scale, scale, scale));
}

Point3 SCTex::PObjRelBox()
{
	Point3 q;
	Point3 p = PObj();
	Box3 b = ObjectBox();
	q.x = 2.0f * (p.x - b.pmin.x) / (b.pmax.x - b.pmin.x) - 1.0f;
	q.y = 2.0f * (p.y - b.pmin.y) / (b.pmax.y - b.pmin.y) - 1.0f;
	q.z = 2.0f * (p.z - b.pmin.z) / (b.pmax.z - b.pmin.z) - 1.0f;
	return q;
}

void SCTex::ScreenUV(Point2& uv, Point2& duv)
{
	uv.x = uvw.x;
	uv.y = uvw.x;
	duv.x = duvw.x;
	duv.y = duvw.y;
}

Point3 SCTex::DPObjRelBox()
{
	return Point3(0, 0, 0);
}

// Bump vectors for UVW: in Camera space
void SCTex::DPdUVW(Point3 dP[3], int chan)
{
	dP[0] = dP[1] = dP[2] = Point3(0, 0, 0);
}

Point3 SCTex::P()
{
	return pt;
}

Point3 SCTex::DP()
{
	return dpt;
}

Point3 SCTex::PObj()
{
	return pt;
}

Point3 SCTex::DPObj()
{
	return dpt;
}

template <>
constexpr int MaxSDK::ColorManagement::ImageLayoutInfo::NumChan<RGBTRIPLE>()
{
	return 3;
}
template <>
constexpr MaxSDK::ColorManagement::ImageLayoutInfo::ChannelType ImageLayoutInfo::ChanType<RGBTRIPLE>()
{
	return ChannelType::type_uint8;
}
template <>
constexpr MaxSDK::ColorManagement::ImageLayoutInfo::ChannelOrder ImageLayoutInfo::ChanOrder<RGBTRIPLE>()
{
	return ChannelOrder::order_bgr;
}

UBYTE* RenderTexMap(Texmap* tex, int w, int h)
{
	float du = 1.0f / float(w);
	float dv = 1.0f / float(h);
	SCTex sc;

	int scanw = ByteWidth(w);
	UBYTE* image = new UBYTE[ByteWidth(w) * h];
	AColor* col = new AColor[w];

	UBYTE* p1;

	sc.scale = 1.0f;
	sc.duvw = Point3(du, dv, 0.0f);
	sc.dpt = sc.duvw;
	sc.uvw.y = 1.0f - 0.5f * dv;

	auto cpm = IColorPipelineMgr::GetInstance();
	auto pipe = cpm ? cpm->GetDefaultViewingPipeline(DisplayViewTarget::kMTL_EDITOR) : nullptr;
	auto conv = pipe ? pipe->GetColorConverter<AColor, RGBTRIPLE>() : nullptr;

	for (int j = 0; j < h; j++)
	{
		sc.scrPos.y = j;
		sc.uvw.x = 0.5f * du;
		p1 = image + (h - j - 1) * scanw;
		for (int i = 0; i < w; i++)
		{
			sc.scrPos.x = i;
			sc.pt = sc.uvw;
			col[i] = tex->EvalColor(sc);
			sc.uvw.x += du;
		}

		if (conv)
		{
			conv->ConvertLine(col, (RGBTRIPLE*)p1, w);
		}

		sc.uvw.y -= dv;
	}

	delete[] col;

	return image;
}
