#pragma once
#ifndef _NIUTILS_H_
#define _NIUTILS_H_

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#  define NOMINMAX
#endif

#include <windows.h>
#include <tchar.h>

#include <cctype>
#include <cmath>
#include <cstdint>
#include <cwctype>
#include <cstring>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <malloc.h>

#include <max.h>
#include <strclass.h>

#if __has_include(<Geom/color.h>)
#  include <Geom/color.h>
#elif __has_include("Geom/color.h")
#  include "Geom/color.h"
#elif __has_include(<geom/color.h>)
#  include <geom/color.h>
#elif __has_include("geom/color.h")
#  include "geom/color.h"
#endif

#include "niflib.h"
#include "obj/NiObject.h"
#include "obj/NiAVObject.h"
#include "obj/NiObjectNET.h"
#include "obj/NiNode.h"
#include "obj/NiTriBasedGeom.h"

#if __has_include(<gen/QuaternionXYZW.h>)
#  include <gen/QuaternionXYZW.h>
#elif __has_include("gen/QuaternionXYZW.h")
#  include "gen/QuaternionXYZW.h"
#endif

#if __has_include(<gen/HalfTexCoord.h>)
#  include <gen/HalfTexCoord.h>
#elif __has_include("gen/HalfTexCoord.h")
#  include "gen/HalfTexCoord.h"
#endif

#if __has_include(<gen/HalfVector3.h>)
#  include <gen/HalfVector3.h>
#elif __has_include("gen/HalfVector3.h")
#  include "gen/HalfVector3.h"
#endif

#include "nif_math.h"
#include "nif_basic_types.h"
#include "nif_io.h"

#ifndef _countof
#  define _countof(x) (sizeof(x) / sizeof((x)[0]))
#endif

#ifdef UNICODE
using tstring = std::wstring;
using tstringstream = std::wstringstream;
using tistringstream = std::wistringstream;
using tostringstream = std::wostringstream;
#else
using tstring = std::string;
using tstringstream = std::stringstream;
using tistringstream = std::istringstream;
using tostringstream = std::ostringstream;
#endif

static constexpr unsigned int IntegerInf = 0x7f7fffffU;
static constexpr unsigned int IntegerNegInf = 0xff7fffffU;

static inline float _BitsToFloat(unsigned int u)
{
	float f = 0.0f;
	std::memcpy(&f, &u, sizeof(f));
	return f;
}

static inline unsigned int _FloatToBits(float f)
{
	unsigned int u = 0;
	std::memcpy(&u, &f, sizeof(u));
	return u;
}

static const float FloatINF = _BitsToFloat(IntegerInf);
static const float FloatNegINF = _BitsToFloat(IntegerNegInf);

namespace Niflib
{
#if !defined(NIUTILS_NIFLIB_HALF_CONVERT_SHIM)
#define NIUTILS_NIFLIB_HALF_CONVERT_SHIM 1
	static inline float ConvertHFloatToFloat(std::uint16_t h)
	{
		const std::uint32_t sign = (std::uint32_t)(h & 0x8000u) << 16;
		const std::uint32_t exp = (std::uint32_t)(h & 0x7C00u) >> 10;
		const std::uint32_t mant = (std::uint32_t)(h & 0x03FFu);

		std::uint32_t fexp = 0;
		std::uint32_t fmant = 0;

		if (exp == 0)
		{
			if (mant == 0)
			{
				return _BitsToFloat(sign);
			}
			std::uint32_t m = mant;
			int e = -1;
			do { e++; m <<= 1; } while ((m & 0x0400u) == 0);
			m &= 0x03FFu;
			fexp = (std::uint32_t)(127 - 15 - e) << 23;
			fmant = (m << 13);
		}
		else if (exp == 0x1Fu)
		{
			fexp = 0xFFu << 23;
			fmant = mant << 13;
			if (fmant == 0)
				return _BitsToFloat(sign | fexp);
			return _BitsToFloat(sign | fexp | fmant | 0x00000001u);
		}
		else
		{
			fexp = (exp + (127 - 15)) << 23;
			fmant = mant << 13;
		}

		return _BitsToFloat(sign | fexp | fmant);
	}

	static inline std::uint16_t ConvertFloatToHFloat(float f)
	{
		const std::uint32_t x = _FloatToBits(f);
		const std::uint16_t sign = (std::uint16_t)((x >> 16) & 0x8000u);
		std::uint32_t mant = x & 0x007FFFFFu;
		int exp = (int)((x >> 23) & 0xFFu) - 127 + 15;

		if (((x >> 23) & 0xFFu) == 0xFFu)
		{
			if (mant != 0)
				return (std::uint16_t)(sign | 0x7C00u | (std::uint16_t)((mant >> 13) ? (mant >> 13) : 1u));
			return (std::uint16_t)(sign | 0x7C00u);
		}

		if (exp <= 0)
		{
			if (exp < -10)
				return sign;
			mant = (mant | 0x00800000u) >> (1 - exp);
			if (mant & 0x00001000u)
				mant += 0x00002000u;
			return (std::uint16_t)(sign | (std::uint16_t)(mant >> 13));
		}

		if (exp >= 31)
		{
			return (std::uint16_t)(sign | 0x7C00u);
		}

		if (mant & 0x00001000u)
		{
			mant += 0x00002000u;
			if (mant & 0x00800000u)
			{
				mant = 0;
				++exp;
				if (exp >= 31)
					return (std::uint16_t)(sign | 0x7C00u);
			}
		}

		return (std::uint16_t)(sign | (std::uint16_t)(exp << 10) | (std::uint16_t)(mant >> 13));
	}
#endif
}

inline LPWSTR A2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars)
{
	if (!lpw || !lpa) return const_cast<LPWSTR>(L"");
	*lpw = L'\0';
	if (0 > mbstowcs(lpw, lpa, nChars)) return const_cast<LPWSTR>(L"");
	return lpw;
}

inline LPSTR W2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars)
{
	if (!lpa || !lpw) return const_cast<LPSTR>("");
	*lpa = '\0';
	if (0 > wcstombs(lpa, lpw, nChars)) return const_cast<LPSTR>("");
	return lpa;
}

#define USES_CONVERSION int _convert; (_convert); LPCWSTR _lpw; (_lpw); LPCSTR _lpa; (_lpa)

#define A2W(lpa) ( \
	((_lpa = (lpa)) == nullptr) ? nullptr : ( \
		_convert = (static_cast<int>(strlen(_lpa)) + 1), \
		A2WHelper((LPWSTR)alloca(_convert * sizeof(WCHAR)), _lpa, _convert)) )

#define W2A(lpw) ( \
	((_lpw = (lpw)) == nullptr) ? nullptr : ( \
		_convert = (static_cast<int>(wcslen(_lpw)) + 1), \
		W2AHelper((LPSTR)alloca(_convert * sizeof(WCHAR)), _lpw, _convert * sizeof(WCHAR))) )

#define A2W_EX(lpa, n) ( \
	((_lpa = (lpa)) == nullptr) ? nullptr : ( \
		_convert = (static_cast<int>(n) + 1), \
		A2WHelper((LPWSTR)alloca(_convert * sizeof(WCHAR)), _lpa, _convert)) )

#define W2A_EX(lpw, n) ( \
	((_lpw = (lpw)) == nullptr) ? nullptr : ( \
		_convert = (static_cast<int>(n) + 1), \
		W2AHelper((LPSTR)alloca(_convert * sizeof(WCHAR)), _lpw, _convert * sizeof(WCHAR))) )

#define A2CW(lpa) ((LPCWSTR)A2W(lpa))
#define W2CA(lpw) ((LPCSTR)W2A(lpw))

std::wstring A2WString(const std::string& str);
std::string  W2AString(const std::wstring& str);

#ifdef UNICODE
#  define A2T(lpa)            A2W(lpa)
#  define W2T(lpw)            static_cast<LPCWSTR>(lpw)
#  define T2A(lpt)            W2A(lpt)
#  define T2W(lpt)            static_cast<LPCWSTR>(lpt)
#  define A2T_EX(lpa, n)      A2W_EX(lpa, n)
#  define W2T_EX(lpw, n)      static_cast<LPCWSTR>(lpw)
#  define T2A_EX(lpt, n)      W2A_EX(lpt, n)
#  define T2W_EX(lpt, n)      static_cast<LPCWSTR>(lpt)
#  define T2AHelper           W2AHelper
#  define T2WHelper(d,s,n)    (s)
#  define A2THelper           A2WHelper
#  define W2THelper(d,s,n)    (s)
#  define T2AString           W2AString
#  define T2WString(s)        (s)
#  define A2TString           A2WString
#  define W2TString(s)        (s)
#else
#  define A2T(lpa)            static_cast<LPCSTR>(lpa)
#  define W2T(lpw)            W2A(lpw)
#  define T2A(lpt)            static_cast<LPCSTR>(lpt)
#  define T2W(lpt)            A2W(lpt)
#  define A2THelper(d,s,n)    static_cast<LPCSTR>(s)
#  define W2THelper           W2AHelper
#  define T2AHelper(d,s,n)    static_cast<LPCSTR>(s)
#  define T2WHelper           A2WHelper
#  define T2AString(s)        (s)
#  define T2WString           A2WString
#  define A2TString(s)        (s)
#  define W2TString           W2AString
#endif

static inline LPTSTR DataForWrite(TSTR& str) { return str.dataForWrite(); }

static inline char* Trim(char*& p)
{
	while (*p && std::isspace((unsigned char)*p)) *p++ = 0;
	char* e = p + std::strlen(p);
	while (e > p)
	{
		unsigned char c = (unsigned char)*(e - 1);
		if (!std::isspace(c)) break;
		*--e = 0;
	}
	return p;
}

static inline wchar_t* Trim(wchar_t*& p)
{
	while (*p && std::iswspace(*p)) *p++ = 0;
	wchar_t* e = p + std::wcslen(p);
	while (e > p)
	{
		wchar_t c = *(e - 1);
		if (!std::iswspace(c)) break;
		*--e = 0;
	}
	return p;
}

struct ltstr
{
	bool operator()(const char* s1, const char* s2) const { return _stricmp(s1, s2) < 0; }
	bool operator()(const std::string& s1, const std::string& s2) const { return _stricmp(s1.c_str(), s2.c_str()) < 0; }
	bool operator()(const std::string& s1, const char* s2) const { return _stricmp(s1.c_str(), s2) < 0; }
	bool operator()(const char* s1, const std::string& s2) const { return _stricmp(s1, s2.c_str()) < 0; }

	bool operator()(const wchar_t* s1, const wchar_t* s2) const { return _wcsicmp(s1, s2) < 0; }
	bool operator()(const std::wstring& s1, const std::wstring& s2) const { return _wcsicmp(s1.c_str(), s2.c_str()) < 0; }
	bool operator()(const std::wstring& s1, const wchar_t* s2) const { return _wcsicmp(s1.c_str(), s2) < 0; }
	bool operator()(const wchar_t* s1, const std::wstring& s2) const { return _wcsicmp(s1, s2.c_str()) < 0; }
};

using NameValueCollectionA = std::map<std::string, std::string, ltstr>;
using KeyValuePairA = std::pair<std::string, std::string>;
using NameValueCollectionW = std::map<std::wstring, std::wstring, ltstr>;
using KeyValuePairW = std::pair<std::wstring, std::wstring>;
using stringlist = std::vector<std::string>;
using wstringlist = std::vector<std::wstring>;
using NameValueListA = std::list<KeyValuePairA>;
using NameValueListW = std::list<KeyValuePairW>;

#ifdef UNICODE
using NameValueCollection = NameValueCollectionW;
using KeyValuePair = KeyValuePairW;
using tstringlist = wstringlist;
using NameValueList = NameValueListW;
#else
using NameValueCollection = NameValueCollectionA;
using KeyValuePair = KeyValuePairA;
using tstringlist = stringlist;
using NameValueList = NameValueListA;
#endif

extern int wildcmp(const char* wild, const char* string);
extern int wildcmpi(const char* wild, const char* string);
extern int wildcmp(const wchar_t* wild, const wchar_t* string);
extern int wildcmpi(const wchar_t* wild, const wchar_t* string);

static inline bool strmatch(const std::string& lhs, const std::string& rhs) { return 0 == _stricmp(lhs.c_str(), rhs.c_str()); }
static inline bool strmatch(const char* lhs, const std::string& rhs) { return 0 == _stricmp(lhs, rhs.c_str()); }
static inline bool strmatch(const std::string& lhs, const char* rhs) { return 0 == _stricmp(lhs.c_str(), rhs); }
static inline bool strmatch(const char* lhs, const char* rhs) { return 0 == _stricmp(lhs, rhs); }

static inline bool strmatch(const std::wstring& lhs, const std::wstring& rhs) { return 0 == _wcsicmp(lhs.c_str(), rhs.c_str()); }
static inline bool strmatch(const wchar_t* lhs, const std::wstring& rhs) { return 0 == _wcsicmp(lhs, rhs.c_str()); }
static inline bool strmatch(const std::wstring& lhs, const wchar_t* rhs) { return 0 == _wcsicmp(lhs.c_str(), rhs); }
static inline bool strmatch(const wchar_t* lhs, const wchar_t* rhs) { return 0 == _wcsicmp(lhs, rhs); }

bool wildmatch(const char* match, const char* value);
bool wildmatch(const std::string& match, const std::string& value);
bool wildmatch(const stringlist& matches, const std::string& value);
bool wildmatch(const wchar_t* match, const wchar_t* value);
bool wildmatch(const std::wstring& match, const std::wstring& value);
bool wildmatch(const wstringlist& matches, const std::wstring& value);

namespace niutils_detail
{
	using PFN_GetPrivateProfileStringA = DWORD(WINAPI*)(LPCSTR, LPCSTR, LPCSTR, LPSTR, DWORD, LPCSTR);
	using PFN_GetPrivateProfileStringW = DWORD(WINAPI*)(LPCWSTR, LPCWSTR, LPCWSTR, LPWSTR, DWORD, LPCWSTR);
	using PFN_GetPrivateProfileIntA = UINT(WINAPI*)(LPCSTR, LPCSTR, INT, LPCSTR);
	using PFN_GetPrivateProfileIntW = UINT(WINAPI*)(LPCWSTR, LPCWSTR, INT, LPCWSTR);
	using PFN_WritePrivateProfileStringA = BOOL(WINAPI*)(LPCSTR, LPCSTR, LPCSTR, LPCSTR);
	using PFN_WritePrivateProfileStringW = BOOL(WINAPI*)(LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR);

	static inline HMODULE Kernel32()
	{
		HMODULE h = ::GetModuleHandleW(L"kernel32.dll");
		if (!h) h = ::LoadLibraryW(L"kernel32.dll");
		return h;
	}

	template <class T>
	static inline T Proc(const char* name)
	{
		HMODULE h = Kernel32();
		if (!h) return nullptr;
		return reinterpret_cast<T>(::GetProcAddress(h, name));
	}

	static inline DWORD GetPrivateProfileStringA_(LPCSTR a, LPCSTR b, LPCSTR c, LPSTR d, DWORD e, LPCSTR f)
	{
		static PFN_GetPrivateProfileStringA p = Proc<PFN_GetPrivateProfileStringA>("GetPrivateProfileStringA");
		return p ? p(a, b, c, d, e, f) : 0;
	}

	static inline DWORD GetPrivateProfileStringW_(LPCWSTR a, LPCWSTR b, LPCWSTR c, LPWSTR d, DWORD e, LPCWSTR f)
	{
		static PFN_GetPrivateProfileStringW p = Proc<PFN_GetPrivateProfileStringW>("GetPrivateProfileStringW");
		return p ? p(a, b, c, d, e, f) : 0;
	}

	static inline UINT GetPrivateProfileIntA_(LPCSTR a, LPCSTR b, INT c, LPCSTR d)
	{
		static PFN_GetPrivateProfileIntA p = Proc<PFN_GetPrivateProfileIntA>("GetPrivateProfileIntA");
		return p ? p(a, b, c, d) : 0;
	}

	static inline UINT GetPrivateProfileIntW_(LPCWSTR a, LPCWSTR b, INT c, LPCWSTR d)
	{
		static PFN_GetPrivateProfileIntW p = Proc<PFN_GetPrivateProfileIntW>("GetPrivateProfileIntW");
		return p ? p(a, b, c, d) : 0;
	}

	static inline BOOL WritePrivateProfileStringA_(LPCSTR a, LPCSTR b, LPCSTR c, LPCSTR d)
	{
		static PFN_WritePrivateProfileStringA p = Proc<PFN_WritePrivateProfileStringA>("WritePrivateProfileStringA");
		return p ? p(a, b, c, d) : FALSE;
	}

	static inline BOOL WritePrivateProfileStringW_(LPCWSTR a, LPCWSTR b, LPCWSTR c, LPCWSTR d)
	{
		static PFN_WritePrivateProfileStringW p = Proc<PFN_WritePrivateProfileStringW>("WritePrivateProfileStringW");
		return p ? p(a, b, c, d) : FALSE;
	}
}

template<typename T>
static inline T GetIniValue(LPCSTR Section, LPCSTR Setting, T Default, LPCSTR iniFileName)
{
	T v{};
	char buffer[1024]{};
	std::stringstream sdef;
	sdef << Default;

	if (0 < niutils_detail::GetPrivateProfileStringA_(Section, Setting, sdef.str().c_str(), buffer, (DWORD)sizeof(buffer), iniFileName))
	{
		std::stringstream sstr(buffer);
		sstr >> v;
		return v;
	}
	return Default;
}

template<typename T>
static inline T GetIniValue(LPCWSTR Section, LPCWSTR Setting, T Default, LPCWSTR iniFileName)
{
	T v{};
	wchar_t buffer[1024]{};
	std::wstringstream sdef;
	sdef << Default;

	if (0 < niutils_detail::GetPrivateProfileStringW_(Section, Setting, sdef.str().c_str(), buffer, (DWORD)_countof(buffer), iniFileName))
	{
		std::wstringstream sstr(buffer);
		sstr >> v;
		return v;
	}
	return Default;
}

template<>
inline int GetIniValue<int>(LPCSTR Section, LPCSTR Setting, int Default, LPCSTR iniFileName)
{
	return (int)niutils_detail::GetPrivateProfileIntA_(Section, Setting, Default, iniFileName);
}

template<>
inline int GetIniValue<int>(LPCWSTR Section, LPCWSTR Setting, int Default, LPCWSTR iniFileName)
{
	return (int)niutils_detail::GetPrivateProfileIntW_(Section, Setting, Default, iniFileName);
}

template<>
inline std::string GetIniValue<std::string>(LPCSTR Section, LPCSTR Setting, std::string Default, LPCSTR iniFileName)
{
	char buffer[1024]{};
	if (0 < niutils_detail::GetPrivateProfileStringA_(Section, Setting, Default.c_str(), buffer, (DWORD)sizeof(buffer), iniFileName))
		return std::string(buffer);
	return Default;
}

template<>
inline std::wstring GetIniValue<std::wstring>(LPCWSTR Section, LPCWSTR Setting, std::wstring Default, LPCWSTR iniFileName)
{
	wchar_t buffer[1024]{};
	if (0 < niutils_detail::GetPrivateProfileStringW_(Section, Setting, Default.c_str(), buffer, (DWORD)_countof(buffer), iniFileName))
		return std::wstring(buffer);
	return Default;
}

#ifdef UNICODE
template<>
inline TSTR GetIniValue<TSTR>(LPCWSTR Section, LPCWSTR Setting, TSTR Default, LPCWSTR iniFileName)
{
	wchar_t buffer[1024]{};
	if (0 < niutils_detail::GetPrivateProfileStringW_(Section, Setting, Default.data(), buffer, (DWORD)_countof(buffer), iniFileName))
		return TSTR(buffer);
	return Default;
}
#else
template<>
inline TSTR GetIniValue<TSTR>(LPCSTR Section, LPCSTR Setting, TSTR Default, LPCSTR iniFileName)
{
	char buffer[1024]{};
	if (0 < niutils_detail::GetPrivateProfileStringA_(Section, Setting, Default.data(), buffer, (DWORD)sizeof(buffer), iniFileName))
		return TSTR(buffer);
	return Default;
}
#endif

template<typename T>
static inline void SetIniValue(LPCSTR Section, LPCSTR Setting, T value, LPCSTR iniFileName)
{
	std::stringstream sstr;
	sstr << value;
	niutils_detail::WritePrivateProfileStringA_(Section, Setting, sstr.str().c_str(), iniFileName);
}

template<typename T>
static inline void SetIniValue(LPCWSTR Section, LPCWSTR Setting, T value, LPCWSTR iniFileName)
{
	std::wstringstream sstr;
	sstr << value;
	niutils_detail::WritePrivateProfileStringW_(Section, Setting, sstr.str().c_str(), iniFileName);
}

template<>
inline void SetIniValue<std::wstring>(LPCWSTR Section, LPCWSTR Setting, std::wstring value, LPCWSTR iniFileName)
{
	niutils_detail::WritePrivateProfileStringW_(Section, Setting, value.c_str(), iniFileName);
}

template<>
inline void SetIniValue<std::string>(LPCSTR Section, LPCSTR Setting, std::string value, LPCSTR iniFileName)
{
	niutils_detail::WritePrivateProfileStringA_(Section, Setting, value.c_str(), iniFileName);
}

#ifdef UNICODE
template<>
inline void SetIniValue<TSTR>(LPCWSTR Section, LPCWSTR Setting, TSTR value, LPCWSTR iniFileName)
{
	niutils_detail::WritePrivateProfileStringW_(Section, Setting, value.data(), iniFileName);
}
#else
template<>
inline void SetIniValue<TSTR>(LPCSTR Section, LPCSTR Setting, TSTR value, LPCSTR iniFileName)
{
	niutils_detail::WritePrivateProfileStringA_(Section, Setting, value.data(), iniFileName);
}
#endif

#ifdef UNICODE
extern TSTR FormatText(const wchar_t* format, ...);
#else
extern TSTR FormatText(const char* format, ...);
#endif
extern std::string  FormatString(const char* format, ...);
extern std::wstring FormatString(const wchar_t* format, ...);

extern stringlist   TokenizeString(LPCSTR str, LPCSTR delims, bool trim = false);
extern wstringlist  TokenizeString(LPCWSTR str, LPCWSTR delims, bool trim = false);
extern stringlist   TokenizeCommandLine(LPCSTR str, bool trim);
extern wstringlist  TokenizeCommandLine(LPCWSTR str, bool trim = false);
extern std::string  JoinCommandLine(stringlist args);
extern std::wstring JoinCommandLine(wstringlist args);

extern std::string  GetIndirectValue(LPCSTR path);
extern std::wstring GetIndirectValue(LPCWSTR path);
extern NameValueCollectionA ReadIniSection(LPCSTR Section, LPCSTR iniFileName);
extern NameValueCollectionW ReadIniSection(LPCWSTR Section, LPCWSTR iniFileName);
extern bool ReadIniSectionAsList(LPCSTR Section, LPCSTR iniFileName, NameValueListA& map);
extern bool ReadIniSectionAsList(LPCWSTR Section, LPCWSTR iniFileName, NameValueListW& map);

extern std::string  ExpandQualifiers(const std::string& src, const NameValueCollectionA& map);
extern std::wstring ExpandQualifiers(const std::wstring& src, const NameValueCollectionW& map);
extern std::string  ExpandEnvironment(const std::string& src);
extern std::wstring ExpandEnvironment(const std::wstring& src);

extern void FindImages(NameValueCollectionA& images, const std::string& rootPath, const stringlist& searchpaths, const stringlist& extensions);
extern void FindImages(NameValueCollectionW& images, const std::wstring& rootPath, const wstringlist& searchpaths, const wstringlist& extensions);

extern void RenameNode(Interface* gi, LPCSTR SrcName, LPCSTR DstName);
extern void RenameNode(Interface* gi, LPCWSTR SrcName, LPCWSTR DstName);

enum PosRotScale
{
	prsPos = 0x1,
	prsRot = 0x2,
	prsScale = 0x4,
	prsDefault = prsPos | prsRot | prsScale,
};

extern void    PosRotScaleNode(INode* n, Point3 p, Quat& q, float s, PosRotScale prs = prsDefault, TimeValue t = 0);
extern void    PosRotScaleNode(Control* c, Point3 p, Quat& q, float s, PosRotScale prs = prsDefault, TimeValue t = 0);
extern void    PosRotScaleNode(INode* n, Matrix3& m3, PosRotScale prs = prsDefault, TimeValue t = 0);
extern void    PosRotScaleNode(Control* c, Matrix3& m3, PosRotScale prs = prsDefault, TimeValue t = 0);
extern Matrix3 GetNodeLocalTM(INode* n, TimeValue t = 0);

extern Niflib::NiNodeRef FindNodeByName(const std::vector<Niflib::NiNodeRef>& blocks, const std::string& name);
extern Niflib::NiNodeRef FindNodeByName(const std::vector<Niflib::NiNodeRef>& blocks, const std::wstring& name);
extern std::vector<Niflib::NiNodeRef> SelectNodesByName(const std::vector<Niflib::NiNodeRef>& blocks, LPCSTR match);
extern std::vector<Niflib::NiNodeRef> SelectNodesByName(const std::vector<Niflib::NiNodeRef>& blocks, LPCWSTR match);
extern int CountNodesByName(const std::vector<Niflib::NiNodeRef>& blocks, LPCSTR match);
extern int CountNodesByName(const std::vector<Niflib::NiNodeRef>& blocks, LPCWSTR match);
extern std::vector<std::string> GetNamesOfNodes(const std::vector<Niflib::NiNodeRef>& blocks);
extern int CountNodesByType(const std::vector<Niflib::NiObjectRef>& blocks, Niflib::Type);

extern INode* FindINode(Interface* i, const std::string& name);
extern INode* FindINode(Interface* i, const std::wstring& name);
extern INode* FindINode(Interface* i, Niflib::NiObjectNETRef node);

struct NodeEquivalence
{
	bool operator()(const Niflib::NiNodeRef& lhs, const Niflib::NiNodeRef& rhs) const
	{
		if (!lhs || !rhs) return lhs < rhs;
		return lhs->GetName() < rhs->GetName();
	}
	bool operator()(const Niflib::NiNodeRef& lhs, const std::string& rhs) const { return lhs && (lhs->GetName() < rhs); }
	bool operator()(const std::string& lhs, const Niflib::NiNodeRef& rhs) const { return rhs ? (lhs < rhs->GetName()) : false; }
};

static inline Niflib::NiNodeRef BinarySearch(std::vector<Niflib::NiNodeRef>& nodes, const std::string& name)
{
	auto pair = std::equal_range(nodes.begin(), nodes.end(), name, NodeEquivalence());
	if (pair.first != pair.second) return *pair.first;
	return Niflib::NiNodeRef();
}

static inline float TODEG(float x) { return x * 180.0f / PI; }
static inline float TORAD(float x) { return x * PI / 180.0f; }

static inline Color TOCOLOR(const Niflib::Color3& c3) { return Color(c3.r, c3.g, c3.b); }
static inline Niflib::Color3 TOCOLOR3(const Color& c) { return Niflib::Color3(c.r, c.g, c.b); }
static inline Niflib::Color3 TOCOLOR3(const Point3& p) { return Niflib::Color3(p.x, p.y, p.z); }
static inline Niflib::Color3 TOCOLOR3(const Niflib::Color4& c4) { return Niflib::Color3(c4.r, c4.g, c4.b); }

static inline Color TOCOLOR(const Niflib::Color4& c4) { return Color(c4.r, c4.g, c4.b); }
static inline Niflib::Color4 TOCOLOR4(const Color& c) { return Niflib::Color4(c.r, c.g, c.b, 1.0f); }
static inline Niflib::Color4 TOCOLOR4(const Niflib::Color3& c3) { return Niflib::Color4(c3.r, c3.g, c3.b, 1.0f); }

static inline Point3 TOPOINT3(const Niflib::Color3& c3) { return Point3(c3.r, c3.g, c3.b); }
static inline Point3 TOPOINT3(const Niflib::Vector3& v) { return Point3(v.x, v.y, v.z); }
static inline Point3 TOPOINT3(const Niflib::Vector4& v) { return Point3(v.x, v.y, v.z); }

static inline Niflib::Vector3 TOVECTOR3(const Point3& v) { return Niflib::Vector3(v.x, v.y, v.z); }
static inline Niflib::Vector3 TOVECTOR3(const Niflib::Vector4& v) { return Niflib::Vector3(v.x, v.y, v.z); }
static inline Niflib::Vector4 TOVECTOR4(const Point3& v, float w = 0.0f) { return Niflib::Vector4(v.x, v.y, v.z, w); }

static inline Quat TOQUAT(const Niflib::Quaternion& q, bool inverse = false)
{
	Quat qt(q.x, q.y, q.z, q.w);
	return (inverse && q.w != FloatNegINF) ? qt.Inverse() : qt;
}

static inline Quat TOQUAT(const Niflib::QuaternionXYZW& q, bool inverse = false)
{
	Quat qt(q.x, q.y, q.z, q.w);
	return (inverse && q.w != FloatNegINF) ? qt.Inverse() : qt;
}

static inline Niflib::Quaternion TOQUAT(const Quat& q, bool inverse = false)
{
	if (inverse && q.w != FloatNegINF) return TOQUAT(q.Inverse(), false);
	return Niflib::Quaternion(q.w, q.x, q.y, q.z);
}

static inline Niflib::QuaternionXYZW TOQUATXYZW(const Niflib::Quaternion& q)
{
	Niflib::QuaternionXYZW qt;
	qt.x = q.x; qt.y = q.y; qt.z = q.z; qt.w = q.w;
	return qt;
}

static inline Niflib::QuaternionXYZW TOQUATXYZW(const Quat& q)
{
	Niflib::QuaternionXYZW qt;
	qt.x = q.x; qt.y = q.y; qt.z = q.z; qt.w = q.w;
	return qt;
}

static inline Matrix3 TOMATRIX3(const Niflib::Matrix44& tm, bool invert = false)
{
	Niflib::Vector3 pos;
	Niflib::Matrix33 rot;
	float scale = 1.0f;
	tm.Decompose(pos, rot, scale);

	Matrix3 m(rot.rows[0].data, rot.rows[1].data, rot.rows[2].data, Point3());
	if (invert) m.Invert();
	m.Scale(Point3(scale, scale, scale));
	m.SetTrans(Point3(pos.x, pos.y, pos.z));
	return m;
}

static inline Niflib::Matrix33 TOMATRIX33(const Matrix3& tm, bool invert = false)
{
	Matrix3 m(tm);
	if (invert) m.Invert();
	return Niflib::Matrix33(
		m.GetRow(0)[0], m.GetRow(0)[1], m.GetRow(0)[2],
		m.GetRow(1)[0], m.GetRow(1)[1], m.GetRow(1)[2],
		m.GetRow(2)[0], m.GetRow(2)[1], m.GetRow(2)[2]);
}

static inline Matrix3 TOMATRIX3(Niflib::Vector3& trans, Niflib::QuaternionXYZW quat, float scale)
{
	Matrix3 tm(true), qm;
	Quat q(quat.x, quat.y, quat.z, quat.w);
	q.MakeMatrix(qm);
	tm.SetTranslate(TOPOINT3(trans));
	tm *= qm;
	tm *= ScaleMatrix(Point3(scale, scale, scale));
	return tm;
}

static inline Matrix3 TOMATRIX3(Niflib::Vector3& trans, Niflib::Quaternion quat, float scale)
{
	Matrix3 tm(true), qm;
	Quat q(quat.x, quat.y, quat.z, quat.w);
	q.MakeMatrix(qm);
	tm.SetTranslate(TOPOINT3(trans));
	tm *= qm;
	tm *= ScaleMatrix(Point3(scale, scale, scale));
	return tm;
}

static inline Niflib::Matrix44 TOMATRIX4(const Matrix3& tm, bool invert = false)
{
	Matrix3 m(tm);
	if (invert) m.Invert();
	Niflib::Matrix33 m3(
		m.GetRow(0)[0], m.GetRow(0)[1], m.GetRow(0)[2],
		m.GetRow(1)[0], m.GetRow(1)[1], m.GetRow(1)[2],
		m.GetRow(2)[0], m.GetRow(2)[1], m.GetRow(2)[2]);
	return Niflib::Matrix44(TOVECTOR3(m.GetTrans()), m3, 1.0f);
}

static inline Point3 GetScale(const Matrix3& mtx)
{
	return Point3(std::fabs(mtx.GetRow(0)[0]), std::fabs(mtx.GetRow(1)[1]), std::fabs(mtx.GetRow(2)[2]));
}

static inline float Average(const Point3& val) { return (val[0] + val[1] + val[2]) / 3.0f; }
static inline float Average(const Niflib::Vector3& val) { return (val.x + val.y + val.z) / 3.0f; }

static inline Niflib::TexCoord TOTEXCOORD(const Niflib::HalfTexCoord& b)
{
	return Niflib::TexCoord(Niflib::ConvertHFloatToFloat(b.u), Niflib::ConvertHFloatToFloat(b.v));
}

static inline Niflib::HalfTexCoord TOHTEXCOORD(const Niflib::TexCoord& b)
{
	Niflib::HalfTexCoord a;
	a.u = Niflib::ConvertFloatToHFloat(b.u);
	a.v = Niflib::ConvertFloatToHFloat(b.v);
	return a;
}

static inline Niflib::Vector3 TOVECTOR3(const Niflib::HalfVector3& b)
{
	return Niflib::Vector3(
		Niflib::ConvertHFloatToFloat(b.x),
		Niflib::ConvertHFloatToFloat(b.y),
		Niflib::ConvertHFloatToFloat(b.z));
}

static inline Niflib::HalfVector3 TOHVECTOR3(const Niflib::Vector3& b)
{
	Niflib::HalfVector3 a;
	a.x = Niflib::ConvertFloatToHFloat(b.x);
	a.y = Niflib::ConvertFloatToHFloat(b.y);
	a.z = Niflib::ConvertFloatToHFloat(b.z);
	return a;
}

template <typename U, typename T>
static inline Niflib::Ref<U> SelectFirstObjectOfType(const std::vector<Niflib::Ref<T>>& objs)
{
	for (auto it = objs.begin(); it != objs.end(); ++it)
	{
		Niflib::Ref<U> obj = DynamicCast<U>(*it);
		if (obj) return obj;
	}
	return Niflib::Ref<U>();
}

template <typename U, typename T>
static inline Niflib::Ref<U> SelectFirstObjectOfType(const std::list<Niflib::Ref<T>>& objs)
{
	for (auto it = objs.begin(); it != objs.end(); ++it)
	{
		Niflib::Ref<U> obj = DynamicCast<U>(*it);
		if (obj) return obj;
	}
	return Niflib::Ref<U>();
}

TSTR PrintMatrix3(Matrix3& m);
TSTR PrintMatrix44(Niflib::Matrix44& m);

extern Modifier* GetOrCreateSkin(INode* node);
extern Modifier* GetSkin(INode* node);
extern TriObject* GetTriObject(Object* o);

extern TSTR GetFileVersion(const char* fileName);
extern TSTR GetFileVersion(const wchar_t* fileName);

template<typename T>
static inline Niflib::Ref<T> CreateNiObject()
{
	return Niflib::StaticCast<T>(T::Create());
}

void CollapseGeomTransform(Niflib::NiTriBasedGeomRef shape);
void CollapseGeomTransforms(std::vector<Niflib::NiTriBasedGeomRef>& shapes);
void FixNormals(std::vector<Niflib::Triangle>& tris, std::vector<Niflib::Vector3>& verts, std::vector<Niflib::Vector3>& norms);

Modifier* GetbhkCollisionModifier(INode* node);
Modifier* CreatebhkCollisionModifier(INode* node, int type, int materialIndex, int layerIndex, unsigned char filter);

void GetIniFileName(char* iniName);
void GetIniFileName(wchar_t* iniName);

Matrix3 GetLocalTM(INode* node);

extern Modifier* GetMorpherModifier(INode* node);
extern Modifier* CreateMorpherModifier(INode* node);
extern void MorpherBuildFromNode(Modifier* mod, int index, INode* target);
extern void MorpherSetName(Modifier* mod, int index, TSTR& name);
extern void MorpherRebuild(Modifier* mod, int index);
extern TSTR MorpherGetName(Modifier* mod, int index);
extern bool MorpherIsActive(Modifier* mod, int index);
extern bool MorpherHasData(Modifier* mod, int index);
extern int  MorpherNumProgMorphs(Modifier* mod, int index);
extern INode* MorpherGetProgMorph(Modifier* mod, int index, int morphIdx);
extern void MorpherGetMorphVerts(Modifier* mod, int index, std::vector<Niflib::Vector3>& verts);

typedef struct EnumLookupType
{
	int value;
	const TCHAR* name;
} EnumLookupType;

extern TSTR   EnumToString(int value, const EnumLookupType* table);
extern LPCTSTR EnumToStringRaw(int value, const EnumLookupType* table);
extern int    StringToEnum(TSTR value, const EnumLookupType* table);
extern int    EnumToIndex(int value, const EnumLookupType* table);

extern TSTR FlagsToString(int value, const EnumLookupType* table);
extern int  StringToFlags(TSTR value, const EnumLookupType* table);

extern unsigned long Crc32Array(const void* data, size_t size);

extern bool GetTexFullName(Texmap* texMap, TSTR& fName);
extern bool GetTexFullName(Texmap* texMap, tstring& fName);

#endif