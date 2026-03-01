/*===========================================================================*\
 | 
 |  FILE:	wM3_io.cpp
 |			Weighted Morpher for MAX R3
 |			Handles loading / saving
 | 
 |  AUTH:   Harry Denholm
 |			Copyright(c) Kinetix 1999
 |			All Rights Reserved.
 |
 |  HIST:	Started 21-9-98	
 |			BitArray native save, 3-3-99 HD
 | 
\*===========================================================================*/

#include "wM3.h"

IOResult morphChannel::Save(ISave* isave)
{
	ULONG nb;
	int j;

	// Size of morph data arrays
	isave->BeginChunk(MR3_POINTCOUNT2);
	isave->Write(&mNumPoints,sizeof(int),&nb);
//	isave->Write(&nmPts,sizeof(int),&nb);
	isave->EndChunk();


	// Save out the morph point data
	isave->BeginChunk(MR3_POINTDATA_MP);
	for(j=0; j<mNumPoints; j++) isave->Write(&mPoints[j],sizeof(Point3),&nb);
	isave->EndChunk();

	isave->BeginChunk(MR3_POINTDATA_MW);
	for(j=0; j<mNumPoints; j++)  isave->Write(&mWeights[j],sizeof(double),&nb);
	isave->EndChunk();

	isave->BeginChunk(MR3_POINTDATA_MD);
	for(j=0; j<mNumPoints; j++) isave->Write(&mDeltas[j],sizeof(Point3),&nb);
	isave->EndChunk();

/*	isave->BeginChunk(MR3_POINTDATA_MO);
	isave->Write(mOptdata,sizeof(int)*nmPts,&nb);
	isave->EndChunk();*/


	// Misc stuff saving
	isave->BeginChunk(MR3_SELARRAY);
	mSel.Save(isave);
	//isave->Write(&mSel,sizeof(BitArray),&nb);
	isave->EndChunk();

	isave->BeginChunk(MR3_NAME);
	isave->WriteWString(mName.data());
	isave->EndChunk();

	isave->BeginChunk(MR3_NONLOCALIZED_NAME);
	isave->WriteWString(mNonLocalizedName.data());
	isave->EndChunk();

	isave->BeginChunk(MR3_PARAMS);
	isave->Write(&mActive,sizeof(BOOL),&nb);
	isave->Write(&mModded,sizeof(BOOL),&nb);
	isave->Write(&mUseLimit,sizeof(BOOL),&nb);
	isave->Write(&mUseSel,sizeof(BOOL),&nb);
	isave->Write(&mSpinmin,sizeof(float),&nb);
	isave->Write(&mSpinmax,sizeof(float),&nb);
	isave->Write(&mActiveOverride,sizeof(BOOL),&nb);
	isave->EndChunk();
	
	isave->BeginChunk(MR3_PROGRESSIVE_PARAMS);
	isave->Write(&mNumProgressiveTargs,sizeof(int),&nb);
	isave->EndChunk();

	isave->BeginChunk(MR3_PROGRESSIVE_CHANNEL_PERCENT);
	isave->Write(&mTargetPercent,sizeof(float),&nb);
	isave->EndChunk();

	isave->BeginChunk(MR3_PROGRESSIVE_CHANNEL_CURVATURE);
	isave->Write(&mCurvature,sizeof(float),&nb);
	isave->EndChunk();

	for (int k = 0; k < mNumProgressiveTargs; k++)
	{
		isave->BeginChunk(MR3_TARGET_CACHE_SUBCHUNK + k);
		mTargetCaches[k].Save(isave);
		isave->EndChunk();
	}


	return IO_OK;
}

IOResult morphChannel::Load(ILoad* iload)
{
	ULONG nb;
	IOResult res = IO_OK;
	int tnPts, tnmPts, j;
	bool selectionLoaded = false;
	bool nonLocalizedNameLoaded = false;
	mNonLocalizedName = _T("");

	while (IO_OK==(res=iload->OpenChunk())) {

		if(iload->CurChunkID()>= MR3_TARGET_CACHE_SUBCHUNK && 
			iload->CurChunkID()< MR3_TARGET_CACHE_SUBCHUNK + MAX_PROGRESSIVE_TARGETS )
		{
			mTargetCaches[(iload->CurChunkID() - MR3_TARGET_CACHE_SUBCHUNK)].Load(iload);
		}

		switch(iload->CurChunkID())  {
			case MR3_POINTCOUNT:
				res = iload->Read(&tnPts,sizeof(int),&nb);
				res = iload->Read(&tnmPts,sizeof(int),&nb);
				mNumPoints = tnPts;
			break;

			case MR3_POINTCOUNT2:
				res = iload->Read(&mNumPoints,sizeof(int),&nb);
			break;

			case MR3_POINTDATA_MP:
			{
				std::vector<Point3>& points = mPoints;
				points.resize(mNumPoints);
				for (j = 0; j < mNumPoints; j++)
				{
					res = iload->Read(&points[j], sizeof(Point3), &nb);
				}
			} break;

			case MR3_POINTDATA_MW:
			{
				std::vector<double>& weights = mWeights;
				weights.resize(mNumPoints);
				for (j = 0; j < mNumPoints; j++)
				{
					res = iload->Read(&weights[j], sizeof(double), &nb);
				}
			} break;
			
			case MR3_POINTDATA_MD:
			{
				std::vector<Point3>& deltas = mDeltas;
				deltas.resize(mNumPoints);
				for (j = 0; j < mNumPoints; j++)
				{
					res = iload->Read(&deltas[j], sizeof(Point3), &nb);
				}
			} break;

			case MR3_SELARRAY:
				selectionLoaded = true;
				mSel.Load(iload);
				//res = iload->Read(&mSel,sizeof(BitArray),&nb);
			break;

			case MR3_NAME:{
				TCHAR *buf;
				res = iload->ReadWStringChunk(&buf);
				mName = TSTR(buf);
			break;}

			case MR3_NONLOCALIZED_NAME:{
				TCHAR *buf;
				res = iload->ReadWStringChunk(&buf);
				mNonLocalizedName = TSTR(buf);
				nonLocalizedNameLoaded = true;
			break;}

			case MR3_PARAMS:
				res = iload->Read(&mActive,sizeof(BOOL),&nb);
				res = iload->Read(&mModded,sizeof(BOOL),&nb);
				res = iload->Read(&mUseLimit,sizeof(BOOL),&nb);
				res = iload->Read(&mUseSel,sizeof(BOOL),&nb);
				res = iload->Read(&mSpinmin,sizeof(float),&nb);
				res = iload->Read(&mSpinmax,sizeof(float),&nb);
				res = iload->Read(&mActiveOverride,sizeof(BOOL),&nb);
			break;

			case MR3_PROGRESSIVE_PARAMS:
				res = iload->Read(&mNumProgressiveTargs,sizeof(int),&nb);
			break;

			case MR3_PROGRESSIVE_CHANNEL_PERCENT:
				res = iload->Read(&mTargetPercent,sizeof(float),&nb); 
			break;

			case MR3_PROGRESSIVE_CHANNEL_CURVATURE:
				res = iload->Read(&mCurvature,sizeof(float),&nb); 
			break;

			case MR3_TARGETCACHE:
			{
				res = iload->Read(&tnPts,sizeof(int),&nb);
				
				TargetCache& tcache = mTargetCaches[tnPts];
				res = iload->Read(&tcache.mNumPoints,sizeof(long),&nb); 
				tcache.mTargetPoints.resize(tcache.mNumPoints);
				for (long k = 0; k < tcache.mNumPoints; k++)
				{
					res = iload->Read(&tcache.mTargetPoints[k], sizeof(Point3), &nb);
				}
			} break;
		}

		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}

	// [dl | 10jan2004] Fix for bug #624608. File saves with Max version < 5 did not save the selection bit array.
	// As a consequence, when such files are loaded, the bit array selection is empty. The code that accesses mSel
	// assumes that mSel has mNumPoints elements. Since the introduction of a DbgAssert in BitArray::operator[],
	// an assert is shown whenever an old file with a morpher modifier is loaded. The fix is to initialize the bit
	// array to the correct count with values all 0, whenever loading an old file. This will preserve the behaviour
	// that has existed sine Max 5 (i.e. all values are assumed to be 0) but will get rid of the DbgAssert.
	if(!selectionLoaded) {
		mSel.SetSize(mNumPoints);
		mSel.ClearAll();
	}

	// This code has been added to properly initialize mNonLocalizedName when loading
	// a Max file that was saved in any version of 3ds Max that is older than 3ds Max 2022
	if (!nonLocalizedNameLoaded &&
		 ((mName == GetString(IDS_EMPTY_CHANNEL)) || (mName == _T("- empty -"))))
	{
		mNonLocalizedName = _T("- empty -");
	}

	return IO_OK;
}

IOResult MorphR3::Load(ILoad *iload)
{
	Modifier::Load(iload);

	ULONG nb;

	IOResult res = IO_OK;
	int mID = 0;
	int maxChunks = 100;
	while (IO_OK==(res=iload->OpenChunk())) {

		if((iload->CurChunkID()>=MR3_MC_SUBCHUNK) && (iload->CurChunkID()<MR3_MC_SUBCHUNK+maxChunks ))
		{
			if (mID >= chanBank.size())
			{
				morphChannel channel;
				channel.mp = this;
				chanBank.push_back(channel);				
			}
			res = chanBank[mID].Load(iload);
			chanBank[mID].mp = this;
			mID++;
		}

		switch (iload->CurChunkID()) {
			case MR3_MC_CHUNKCOUNT:
				{
					int chunkSize =0;
					res = iload->Read(&chunkSize,sizeof(chunkSize), &nb);
					maxChunks = chunkSize;
					chanBank.resize(chunkSize);
				}
			break;
			case MR3_FILE_VERSION:
				res = iload->Read(&mFileVersion,sizeof(float), &nb);
			break;

			case MR3_MARKERNAME:
				markerName.Load(iload);
				break;

			case MR3_MARKERINDEX:
				int n;
				res = iload->Read(&n,sizeof(int), &nb);
				markerIndex.SetCount(n);

				for(int x=0;x<n;x++)
				{
					res = iload->Read(&markerIndex[x],sizeof(int), &nb);
				}
				break;
		}

		iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}	

	iload->RegisterPostLoadCallback(new MorphR3PostLoadCallback(this));

	return IO_OK;
}

IOResult MorphR3::Save(ISave *isave)
{
	Modifier::Save(isave);

	ULONG nb;
	int n;

	float tempversion = MR3_MORPHERVERSION;

	isave->BeginChunk(MR3_FILE_VERSION);
	isave->Write(&tempversion, sizeof(float), &nb);
	isave->EndChunk();

	isave->BeginChunk(MR3_MC_CHUNKCOUNT);
	int  channelCount = (int)chanBank.size();
	isave->Write(&channelCount, sizeof(int), &nb);
	isave->EndChunk();
	

	for(int i=0;i<chanBank.size();i++)
	{
		isave->BeginChunk(MR3_MC_SUBCHUNK+i);
		chanBank[i].Save(isave);
		isave->EndChunk();	
	}

	isave->BeginChunk(MR3_MARKERNAME);
	markerName.Save(isave);
	isave->EndChunk();

	isave->BeginChunk(MR3_MARKERINDEX);
	n = markerIndex.Count();
	isave->Write(&n,sizeof(int), &nb);

	for(int x=0;x<n;x++)
	{
		isave->Write(&markerIndex[x],sizeof(int), &nb);
	}
	isave->EndChunk();

	return IO_OK;
}