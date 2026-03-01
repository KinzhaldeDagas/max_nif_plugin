//////////////////////////////////////////////////////////////////////////////
//
//  Copyright 2009 Autodesk, Inc.  All rights reserved.
//  Copyright 2003 Character Animation Technologies.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//
//////////////////////////////////////////////////////////////////////////////

#include <Windows.h>
#include <shellapi.h>

#include "Max.h"
#include "CatDotIni.h"
#include "IniFile.h"

#include "../CATControls/CATFilePaths.h"
#include "../CATControls/resource.h"

#include <Util/FileMutexObject.h>

#define CAT_INI_FILE _T("CAT\\cat.ini")

// Get strings from the String Table
extern TCHAR *GetString(int id); // From CATControls/DllEntry.cpp

//////////////////////////////////////////////////////////////////////
// Defaults for the [Paths] group.
//
static TCHAR pTempPathBuffer[MAX_PATH];

const TCHAR *DefaultRigPresetPath() {
	_stprintf_s(pTempPathBuffer, _countof(pTempPathBuffer), _T("%s\\CAT\\CATRigs"), GetCOREInterface()->GetDir(APP_USER_SETTINGS_DIR).data());
	return pTempPathBuffer;
}

const TCHAR *DefaultMotionPresetPath() {
	_stprintf_s(pTempPathBuffer, _countof(pTempPathBuffer), _T("%s\\CAT\\MotionPresets"), GetCOREInterface()->GetDir(APP_USER_SETTINGS_DIR).data());
	return pTempPathBuffer;
}

const TCHAR *DefaultHandPosePresetPath() {
	_stprintf_s(pTempPathBuffer, _countof(pTempPathBuffer), _T("%s\\CAT\\HandPosePresets"), GetCOREInterface()->GetDir(APP_USER_SETTINGS_DIR).data());
	return pTempPathBuffer;
}

const TCHAR *DefaultClipsPath() {
	_stprintf_s(pTempPathBuffer, _countof(pTempPathBuffer), _T("%s\\CAT\\Clips"), GetCOREInterface()->GetDir(APP_USER_SETTINGS_DIR).data());
	return pTempPathBuffer;
}

const TCHAR *DefaultPosesPath() {
	_stprintf_s(pTempPathBuffer, _countof(pTempPathBuffer), _T("%s\\CAT\\Poses"), GetCOREInterface()->GetDir(APP_USER_SETTINGS_DIR).data());
	return pTempPathBuffer;
}


CatDotIni::PathMap CatDotIni::sPathMap{};

CatDotIni::PathMap::key_type CatDotIni::ToPathMapKey(const TCHAR *szGroup, const TCHAR *szKey)
{
	std::wstring key;
	key.append(szGroup).append(_T("_")).append(szKey);
	return key;
}

void CatDotIni::InitializePathMap()
{
	if (sPathMap.empty())
	{
		sPathMap[ToPathMapKey(KEY_RIG_PRESET_PATH)] = DefaultRigPresetPath();
		sPathMap[ToPathMapKey(KEY_MOTION_PRESET_PATH)] = DefaultMotionPresetPath();
		sPathMap[ToPathMapKey(KEY_HAND_POSE_PRESET_PATH)] = DefaultHandPosePresetPath();
		sPathMap[ToPathMapKey(KEY_CLIPS_PATH)] = DefaultClipsPath();
		sPathMap[ToPathMapKey(KEY_POSES_PATH)] = DefaultPosesPath();
	}
}

bool CatDotIni::InitializeUserSettingsPath()
{
	MaxSDK::Util::Path destination(GetUserSettingsPath());

	MaxSDK::Util::FileMutexObject fileLock(destination.GetCStr());

	if (!destination.Exists())
	{
		MaxSDK::Util::Path source(GetCOREInterface()->GetDir(APP_PRESETS_DIR));
		source.Append(MaxSDK::Util::Path(_T("CAT")));
		if (!source.Exists())
		{
			TSTR message = GetString(IDS_ERR_CATPRESETS_INIT);
			MaxSDK::MaxMessageBox(NULL, message, GetString(IDS_ERROR), MB_OK | MB_ICONSTOP);
			return false;
		}

		// this init is important, since SHFILEOPSTRUCT will be expecting a double-NULL terminating chars
		TCHAR fromBuf[MAX_PATH] = { 0 };
		TCHAR toBuf[MAX_PATH] = { 0 };

		_tcscpy_s(fromBuf, _countof(fromBuf), source.GetCStr());
		_tcscpy_s(toBuf, _countof(toBuf), destination.GetCStr());

		fromBuf[_tcsnlen(fromBuf, _countof(fromBuf)) + 1] = _T('\0');
		toBuf[_tcsnlen(toBuf, _countof(toBuf)) + 1] = _T('\0');

		SHFILEOPSTRUCT fileOpStruct;
		fileOpStruct.hwnd = NULL;
		fileOpStruct.wFunc = FO_COPY;
		fileOpStruct.pFrom = fromBuf;  // double-NULL delimited buf
		fileOpStruct.pTo = toBuf;  // double-NULL delimited buf	
		fileOpStruct.fFlags =
			(FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOCOPYSECURITYATTRIBS |
				FOF_NOERRORUI | FOF_SILENT);
		fileOpStruct.fAnyOperationsAborted = FALSE;
		fileOpStruct.hNameMappings = NULL;
		fileOpStruct.lpszProgressTitle = NULL;

		int copyResult = SHFileOperation(&fileOpStruct);
		if (copyResult)
		{
			TSTR message = GetString(IDS_ERR_CATPRESETS_INIT);
			MaxSDK::MaxMessageBox(NULL, message, GetString(IDS_ERROR), MB_OK | MB_ICONSTOP);
			return false;
		}
	}
	return true;
}


//
// Constructs the CatDotIni class and automatically loads the file.
//
CatDotIni::CatDotIni()
{
	CatDotIni::InitializePathMap();

	TCHAR szIniFile[MAX_PATH] = { 0 };
	_stprintf_s(szIniFile, _countof(szIniFile), _T("%s\\%s"), GetCOREInterface()->GetDir(APP_USER_SETTINGS_DIR).data(), CAT_INI_FILE);
	bDirty = false;

	ini = new IniFile(szIniFile);
	DbgAssert(ini);
}

//
// If there has been any changes since
CatDotIni::~CatDotIni()
{
	if (Dirty()) Write();
	delete ini;
}

//
// These functions read and write the entire .ini file.
//
bool CatDotIni::Reload()
{
	return ini->Load();
}

bool CatDotIni::Write()
{
	return ini->Save();
}

//
// Returns true if anything has changed since loading/saving.
//
bool CatDotIni::Dirty() const
{
	return ini->Dirty();
}

void CatDotIni::ClearDirty()
{
	ini->ClearDirty();
}

//
// Returns true if the given key exists within the given group
//
bool CatDotIni::Exists(const TCHAR *szGroup, const TCHAR *szKey) const
{
	int nGroup = ini->FindGroup(szGroup);
	if (nGroup == -1) return false;
	if (ini->FindKey(nGroup, szKey) == -1) return false;
	return true;
}

//
// This returns the value of the requested key, or NULL if the key
// does not exist.
//
const TCHAR *CatDotIni::Get(const TCHAR *szGroup, const TCHAR *szKey, const TCHAR *szDefault/*=""*/)
{
	// MAXX-59875 - Initialize the CAT user settings folder on first use,
	// i.e. on first access of one of the CatDotIni path keys
	if (sPathMap.find(ToPathMapKey(szGroup, szKey)) == sPathMap.end())
	{
		if (!InitializeUserSettingsPath())
		{
			return nullptr;
		}
	}

	const TCHAR *szValue = ini->GetKey(szGroup, szKey);
	if (!szValue) {
		Set(szGroup, szKey, szDefault);
		return ini->GetKey(szGroup, szKey);
	}
	return szValue;
}

//
// These functions retrieve the value of the requested key and
// convert it to the required type.  If the key does not exist,
// the default value for each type is returned.  This is usually
// a representation of zero.
//
int CatDotIni::GetInt(const TCHAR *szGroup, const TCHAR *szKey, const TCHAR *szDefault/*=""*/)
{
	const TCHAR *szValue = Get(szGroup, szKey, szDefault);
	if (!szValue) return 0;
	return _ttoi(szValue);
}

bool CatDotIni::GetBool(const TCHAR *szGroup, const TCHAR *szKey, const TCHAR *szDefault/*=""*/)
{
	const TCHAR *szValue = Get(szGroup, szKey, szDefault);
	if (!_tcscmp(szValue, _T("")) || !_tcscmp(szValue, _T("0"))) return false;
	return true;
}

float CatDotIni::GetFloat(const TCHAR *szGroup, const TCHAR *szKey, const TCHAR *szDefault/*=""*/)
{
	const TCHAR *szValue = Get(szGroup, szKey, szDefault);
	if (!szValue) return 0.0f;
	float fValue = (float)_tstof_l(szValue, GetCNumericLocale());
	return fValue;
}

double CatDotIni::GetDouble(const TCHAR *szGroup, const TCHAR *szKey, const TCHAR *szDefault/*=""*/)
{
	const TCHAR *szValue = Get(szGroup, szKey, szDefault);
	if (!szValue) return 0.0;
	float dValue = (float)_tstof_l(szValue, GetCNumericLocale());
	return dValue;
}

//
// These functions set the value of a key.  If the key does not exist
// it is created (along with the group if that does not exist).
// Note that all values are converted to strings, so in the case of
// floating point numbers, some precision may be lost.
//
void CatDotIni::Set(const TCHAR *szGroup, const TCHAR *szKey, const TCHAR *szValue)
{
	bDirty = true;
	ini->SetKey(szGroup, szKey, szValue);
}

void CatDotIni::SetInt(const TCHAR *szGroup, const TCHAR *szKey, int nValue)
{
	static TCHAR szBuffer[33];
	Set(szGroup, szKey, _itot(nValue, szBuffer, 10));
}

void CatDotIni::SetBool(const TCHAR *szGroup, const TCHAR *szKey, bool bValue)
{
	if (bValue) Set(szGroup, szKey, _T("1"));
	else Set(szGroup, szKey, _T("0"));
}

void CatDotIni::SetFloat(const TCHAR *szGroup, const TCHAR *szKey, float fValue)
{
	static TCHAR szBuffer[50];
	_stprintf_s(szBuffer, _countof(szBuffer), _T("%e"), (double)fValue);
	Set(szGroup, szKey, szBuffer);
}

void CatDotIni::SetDouble(const TCHAR *szGroup, const TCHAR *szKey, double dValue)
{
	static TCHAR szBuffer[50];
	_stprintf_s(szBuffer, _countof(szBuffer), _T("%e"), dValue);
	Set(szGroup, szKey, szBuffer);
}

const IniFile *CatDotIni::GetIniFile() const
{
	return ini;
}
