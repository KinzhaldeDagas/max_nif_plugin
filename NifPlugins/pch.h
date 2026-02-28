#pragma once

// Precompiled Header (3ds Max 2026 / VS2022 / x64)

// Legacy warning from old implicit bool conversions
#pragma warning(disable : 4800)

//------------------------------
// Standard Library
//------------------------------
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <deque>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

//------------------------------
// Windows
//------------------------------
#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#  define NOMINMAX
#endif

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <direct.h>
#include <io.h>
#include <shellapi.h>
#include <shlwapi.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shlwapi.lib")

//------------------------------
// Local ASSERT (do not fight Max SDK's Assert macros; keep lightweight)
//------------------------------
#ifndef ASSERT
#  ifdef _DEBUG
#    include <crtdbg.h>
#    define ASSERT _ASSERTE
#  else
#    define ASSERT(exp) ((void)0)
#  endif
#endif

//------------------------------
// 3ds Max SDK (2026+ only)
//------------------------------
#include <max.h>
#include "MAX_MemDirect.h"

#include <istdplug.h>
#include <iparamb2.h>
#if __has_include(<iparamm2.h>)
#  include <iparamm2.h>
#endif

#ifdef USE_BIPED
#  include <cs/BipedApi.h>
#endif

#include <plugapi.h>
#include <triobj.h>
#include <bitmap.h>
#include <modstack.h>
#include <iskin.h>
#include <strclass.h>

#include "objectParams.h"

// Max SDK occasionally defines these; NIF/NvTriStrip code may use same tokens
#ifdef ALPHA_NONE
#  undef ALPHA_NONE
#endif
#ifdef DECAY_NONE
#  undef DECAY_NONE
#endif

//------------------------------
// Niflib
//------------------------------
// Niflib headers use legacy unqualified `byte` in some versions, which clashes
// with `std::byte` when both are visible. Map bare `byte` tokens only while
// including niflib headers, then restore normal behavior for the Max SDK.
typedef unsigned char niflib_legacy_byte;
#define byte niflib_legacy_byte
#include "niflib.h"
#undef byte

// Generated headers are layout-dependent across niflib forks/builds.
// Gate them to prevent hard build breaks in the PCH.
#if __has_include(<gen/BSVertexData.h>)
#  include <gen/BSVertexData.h>
#elif __has_include(<niflib/gen/BSVertexData.h>)
#  include <niflib/gen/BSVertexData.h>
#elif __has_include(<BSVertexData.h>)
#  include <BSVertexData.h>
#else
struct BSVertexData;
#endif

#if __has_include(<gen/BSSkinBoneTrans.h>)
#  include <gen/BSSkinBoneTrans.h>
#elif __has_include(<niflib/gen/BSSkinBoneTrans.h>)
#  include <niflib/gen/BSSkinBoneTrans.h>
#elif __has_include(<BSSkinBoneTrans.h>)
#  include <BSSkinBoneTrans.h>
#else
struct BSSkinBoneTrans;
#endif

#include "obj/NiObject.h"
#include "obj/NiNode.h"
#include "obj/NiTriShape.h"
#include "obj/NiTriShapeData.h"
#include "obj/NiTriStrips.h"
#include "obj/NiTriStripsData.h"
#include "obj/NiMaterialProperty.h"
#include "obj/NiTexturingProperty.h"
#include "obj/NiSourceTexture.h"
#include "obj/NiExtraData.h"
#include "obj/BSBound.h"
#include "obj/NiSkinData.h"
#include "obj/NiSkinInstance.h"
#include "obj/NiSkinPartition.h"
#include "obj/NiLight.h"
#include "obj/BsxFlags.h"
#include "obj/NiStringExtraData.h"

#include "obj/bhkCollisionObject.h"
#include "obj/bhkRigidBody.h"
#include "obj/bhkRigidBodyT.h"
#include "obj/bhkNiTriStripsShape.h"
#include "obj/bhkBoxShape.h"
#include "obj/bhkSphereShape.h"
#include "obj/bhkCapsuleShape.h"

#include "niutils.h"
#include "AppSettings.h"

//------------------------------
// TriStripper / NvTriStrip
//------------------------------
#ifdef max
#  undef max
#endif
#ifdef min
#  undef min
#endif

#include "NvTriStrip/NvTriStrip.h"
#include "TriStripper/tri_stripper.h"

//------------------------------
// Project
//------------------------------
#include "NifPlugins.h"
