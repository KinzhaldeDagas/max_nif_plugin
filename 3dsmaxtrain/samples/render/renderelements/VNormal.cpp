//**************************************************************************/
// Copyright (c) 2020 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// DESCRIPTION: Utility to compute the normal and tangent basis for rendering
// AUTHOR: Autodesk Inc.
//***************************************************************************/

#include "VNormal.h"
#include <Graphics/NormalMappingManager.h>
#include <intrin.h>

//-----------------------------------------------------------------------------
//--- Constants

//Defines an invalid or "null" map channel number
#define NULL_MESHMAP (-(NUM_HIDDENMAPS+1))

Point3 zeroNormal(0,0,0);
Point3 stubNormal(0,0,1);
float stubTangentSign(1.0f);
TangentBasis zeroTangentBasis = ConstructTangentBasis( Point3(0,0,0), Point3(0,0,0), Point3(0, 0, 0));
TangentBasis stubTangentBasis = ConstructTangentBasis( Point3(1,0,0), Point3(0,1,0), Point3(0, 0, 1));

//-----------------------------------------------------------------------------
//--- General utilities

// Atomic operation:
// Sets (*p)+=a, and returns the previous value of (*p)
DWORD LockXAdd( DWORD* p, DWORD a )
{
	return _InterlockedExchangeAdd( reinterpret_cast< LONG * >( p ), a );
}

// Atomic operation:
// If (*p)==a, then ... sets (*p)=b and returns true
// Else           ... sets (*q)=(*p) and returns false
DWORD LockCmpXChg( DWORD* p, DWORD Exchange, DWORD Comperand )
{
	return _InterlockedCompareExchange( reinterpret_cast< LONG * >( p ), Exchange, Comperand );
}

// Atomic operation: (*p)++
void LockInc( DWORD* p )
{
	_InterlockedIncrement( reinterpret_cast< LONG* >( p ) );
}

// Atomic operation: (*p)--
void LockDec( DWORD* p )
{
	_InterlockedDecrement( reinterpret_cast< LONG* >( p ) );
}

// Atomic operation: (*p) &= a
void LockAnd( DWORD* p, DWORD a )
{
	_InterlockedAnd( reinterpret_cast< LONG* >( p ), a );
}

//===========================================================================
//
// Class ReaderWriterLock
//
//===========================================================================

#define READER_BITS 0x7FFFFFFF
#define WRITER_BITS 0x80000000

void ReaderWriterLock::EnterRead()
{
	// Increments the count, and get the previous count (atomic operation)
	int checkBits = LockXAdd( &bits, 1 );
	while ( checkBits & WRITER_BITS )
	{
		Sleep( 0 ); //SwitchToThread();
		checkBits = bits;
	}
}

void ReaderWriterLock::ExitRead()
{
	DbgAssert( ( bits & READER_BITS ) != 0 );
	LockDec( &bits ); //bits-- (atomic operation)
}

void ReaderWriterLock::EnterWrite()
{
	DWORD prevBits;
	while ( TRUE )
	{
		// If bits==0, sets bits=WRITER_BITS. Returns original value of bits (atomic operation)
		prevBits = LockCmpXChg( &bits, WRITER_BITS, 0 );
		if ( prevBits == 0 ) break; // success
		else
			Sleep( 0 ); //SwitchToThread();
	}
}

void ReaderWriterLock::ExitWrite()
{
	DbgAssert( ( bits & WRITER_BITS ) != 0 );
	LockAnd( &bits, ( ~WRITER_BITS ) ); //bits &= ~WRITER_BITS (atomic operation)
}

void memswp( void* a, void* b, int size ) {
	char* temp = new char[size];
	memcpy_s( temp, size, a, size );
	memcpy_s( a, size, b, size );
	memcpy_s( b, size, temp, size );
	delete[] temp;
}

TangentBasis ConstructTangentBasis( Point3 uBasis, Point3 vBasis, Point3 nBasis ) {
	TangentBasis retVal;
	retVal.uBasis=uBasis, retVal.vBasis=vBasis, retVal.nBasis = nBasis;
	return retVal;
}


//===========================================================================
//
// Class VertFaceLookup
//
// Indicates the faces bordering each vertex in a mesh
// Helper for VertFaceGrouping
//
//===========================================================================

VertFaceLookup::VertFaceLookup() {
	vertFaces = NULL, vertFaceIndex = NULL;
}

VertFaceLookup::~VertFaceLookup() {
	if( vertFaces!=NULL )		delete[] vertFaces;
	if( vertFaceIndex!=NULL )	delete[] vertFaceIndex;
	vertFaces = NULL, vertFaceIndex = NULL;
}

int VertFaceLookup::GetCount( int vertIndex ) {
	return vertFaceIndex[ vertIndex+1 ] - vertFaceIndex[ vertIndex ];
}

VertFace VertFaceLookup::GetVertFace( int vertIndex, int i ) {
	int index = i + vertFaceIndex[ vertIndex ];
	return vertFaces[ index ];
}

void VertFaceLookup::Init( Mesh& mesh ) {
	int numVerts = mesh.numVerts;
	int numFaces = mesh.numFaces;
	int i, f, v, fv; //loop variables

	// 1. Calculate number of faces per vert
	vertFaceIndex = new int[ numVerts + 1 ];
	int* vertFaceCount = vertFaceIndex;
	int  vertFaceTotal = 0;
	if( numVerts>0 ) memset( vertFaceCount, 0, numVerts*sizeof(int) );
	for( f=0; f<numFaces; f++ ) {
		int numFaceVerts = 3; //GetNumFaceVerts(i);
		for( int fv=0; fv<numFaceVerts; fv++ ) {
			int vertIndex = mesh.faces[ f ].getVert( fv );
			vertFaceCount[ vertIndex ]++;
			vertFaceTotal++;
		}
	}


	// 2. Calculate the vert -> vertFace indexing.
	// Each vert will have a contiguous set of vertFaces in the vertFace array;
	// calculate the index of the first entry in the array for each vert
	int vertFaceCurIndex = 0;
	for( v=0; v<numVerts; v++ ) {
		int temp = vertFaceCount[v];
		vertFaceIndex[v] = vertFaceCurIndex;
		vertFaceCurIndex += temp;
	}
	vertFaceIndex[numVerts] = vertFaceCurIndex;


	// 3. Allocate the vert -> face lookup (the vertFace array)
	vertFaces = new VertFace[ vertFaceTotal>0? vertFaceTotal:1 ];
	if( vertFaceTotal>0 ) memset( vertFaces, 0xFFFFFFFF, vertFaceTotal * sizeof(VertFace) );


	// 4. Initialize the vert -> face lookup
	for( f=0; f<numFaces; f++ ) {
		int numFaceVerts = 3; //GetNumFaceVerts(i);
		for( fv=0; fv<numFaceVerts; fv++ ) {
			int vertIndex = mesh.faces[ f ].getVert( fv );
			int vertFaceCur =  vertFaceIndex[ vertIndex ];
			int vertFaceNext = vertFaceIndex[ vertIndex+1 ];
			for( i=vertFaceCur; i<vertFaceNext; i++ )
				if( vertFaces[i].faceIndex==0xFFFFFFFF ) break;
			DbgAssert( i!=vertFaceNext );
			vertFaces[i].faceIndex = f;
			vertFaces[i].faceVertIndex = fv;
		}
	}
}


//===========================================================================
//
// Class VertFaceGrouping
//
// Indicates the faces bordering each vertex in a mesh
//
//===========================================================================


void VertFaceGrouping::Init( Mesh& mesh, int mapChannel ) {
	int numFaces = mesh.getNumFaces();
	int numVerts = mesh.getNumVerts();

	MeshData meshData;
	meshData.Init( mesh, mapChannel );

	VertFaceLookup lookup;
	lookup.Init( mesh );

	faceBits.SetCount( numFaces );
	groupCounts.SetCount( numVerts );
	if( numFaces>0 ) memset( faceBits.Addr(0), 0, numFaces*sizeof(WORD) );
	if( numVerts>0 ) memset( groupCounts.Addr(0), 0, numVerts*sizeof(WORD) );

	Tab<FaceData> faceData;
	faceData.SetCount( numFaces );
	for( int f=0; f<numFaces; f++ ) {
		faceData[f].Init( meshData, f );
		if( faceData[f].IsFaceted() ) faceBits[f] = BIT_ISFACETED;
	}

	for( int v=0; v<numVerts; v++ ) {
		InitVert( faceData, lookup, v );
	}
	}

void VertFaceGrouping::Free() {
	faceBits.SetCount(0);
	groupCounts.SetCount(0);
}

int VertFaceGrouping::GetGroup( int faceIndex, int faceVertIndex ) {
	WORD bits = faceBits[ faceIndex ];
	switch( faceVertIndex ) {
	case 0:		return (0x001F & (bits));
	case 1:		return (0x001F & (bits>>5));
	case 2:		return (0x001F & (bits>>10));
	}
	return -1; //error
}

void VertFaceGrouping::SetGroup( int faceIndex, int faceVertIndex, int group ) {
	WORD& bits = faceBits[ faceIndex ];
	group = group & 0x001F;
	switch( faceVertIndex ) {
	case 0:		bits = (bits & 0xFFE0) | (group);			break;
	case 1:		bits = (bits & 0xFC1F) | (group<<5);		break;
	case 2:		bits = (bits & 0x83FF) | (group<<10);		break;
	}
}

WORD VertFaceGrouping::GetGroupCount( int vertIndex ) {return groupCounts[vertIndex];}

void VertFaceGrouping::Compact() {
	groupCounts.Resize(0);
}

void VertFaceGrouping::InitVert( Tab<FaceData>& faceData, VertFaceLookup& lookup, int vertIndex ) {
	int faceCount = lookup.GetCount( vertIndex );

	GroupData** groupDataList = new GroupData*[ faceCount ];
	int groupCount = 0;

	for( int i=0; i<faceCount; i++ ) {
		VertFace vertFace = lookup.GetVertFace( vertIndex, i );
		int f = vertFace.faceIndex;
		int fv = vertFace.faceVertIndex;

		if( faceData[f].IsFaceted() ) continue;
		GroupData& groupDataCur = faceData[f][fv];

      int j;
		for( j=0; j<groupCount; j++ )
			if( groupDataList[j]->IsMatch( groupDataCur ) ) {
				groupDataList[j]->Merge( groupDataCur );
				break;
			}

		if( j==32 ) j=31; //NOTE: limit 32 groups per vert

		if( j==groupCount ) {
			groupDataList[j] = &groupDataCur;
			groupCount++;
		}

		SetGroup( f, fv, j );
	}
	groupCounts[ vertIndex ] = groupCount;

	delete[] groupDataList;
}

//===========================================================================
//
// Class MeshData and GroupData; subclasses of VertFaceGrouping
// Helper classes
//
//===========================================================================

void VertFaceGrouping::MeshData::Init( Mesh& mesh, int mapChannel ) {
	geomFaces	 = mesh.faces;
	mapFaces	 = mesh.mapFaces( mapChannel );
	normalSpec	 = mesh.GetSpecifiedNormals();
	if( (normalSpec!=NULL) && (normalSpec->GetNumNormals()==0) )
		normalSpec=NULL;

	// Initialize the mapBackfacing bits
	int numFaces = mesh.getNumFaces();
	mapBackfacing.SetSize( numFaces );
	mapBackfacing.ClearAll();
	UVVert* tv = mesh.mapVerts( mapChannel );
	if( (mapFaces!=NULL) && (tv!=NULL) ) {
		Point3 mapNormal;
		for( int f=0; f<numFaces; f++ ) {
			DWORD* tf = mapFaces[f].t;
			mapNormal = FNormalize( (tv[tf[1]] - tv[tf[0]]) ^ (tv[tf[2]] - tv[tf[1]]) );
			mapBackfacing.Set( f, (mapNormal.z<0? TRUE:FALSE) );
		}
	}
}

void VertFaceGrouping::GroupData::Init( MeshData& meshData, int faceIndex, int faceVertIndex ) {
	smGroup = meshData.geomFaces[faceIndex].smGroup;
	if( meshData.mapFaces!=NULL ) {
		mapVert = meshData.mapFaces[faceIndex].t[faceVertIndex];
		if( meshData.mapBackfacing[faceIndex] )
			mapVert |= 0x80000000; //backfacing flag held in upper bit
		}
	else mapVert = -1;
	if( meshData.normalSpec!=NULL )
		 normalSpec = meshData.normalSpec->GetNormal( faceIndex, faceVertIndex );
	else normalSpec.Set(0,0,0);
	}

BOOL VertFaceGrouping::GroupData::IsMatch( GroupData& that ) {
	if( !(this->smGroup & that.smGroup) )		return FALSE;
	if( this->normalSpec != that.normalSpec )	return FALSE;
	//note: backfacing flag held in upper bit of mapVert value
	if( this->mapVert != that.mapVert )			return FALSE;
	return TRUE;
		}

void VertFaceGrouping::GroupData::Merge( GroupData& that ) {
	this->smGroup |= that.smGroup;
	}

void VertFaceGrouping::FaceData::Init( MeshData& meshData, int faceIndex ) {
	groupData[0].Init( meshData, faceIndex, 0 );
	groupData[1].Init( meshData, faceIndex, 1 );
	groupData[2].Init( meshData, faceIndex, 2 );
}

BOOL VertFaceGrouping::FaceData::IsFaceted() {
	DWORD smGroup = groupData[0].smGroup; //arbitrarily use the first item
	DbgAssert( ((groupData[1].smGroup & smGroup) || (groupData[1].smGroup ==0 ))
		&& ((groupData[2].smGroup & smGroup) || (groupData[2].smGroup==0 )) );
	return (smGroup==0? TRUE:FALSE);
		}

//===========================================================================
//
// VNormalChannel
//
// Holds a normal vector and related data at each vertex & smoothing group;
// that is, one piece of data per vertex per smoothing group at the vertex.
//
//===========================================================================

VNormalChannel::VNormalChannel() {
	valid = validTangentBasis = validNormals = FALSE;
	mapChannel = 0;
	numItems = 0;
}

void VNormalChannel::Init( Mesh& mesh, int mapChannel ) {
	int numFaces = mesh.getNumFaces(), numVerts = mesh.getNumVerts();
	vertOffset.SetCount( numVerts );
	vertBase.SetCount( 1+(numVerts>>8) );
	faceBase.SetCount( 1+(numFaces>>8) );
	this->mapChannel = mapChannel;

	// 1. Calculate the surface normal groups for each vert
	grouping.Init( mesh, mapChannel );

	// The array of surface normals will have two sections;
	// first the vert normals, then the face normals for faceted faces.

	// 2. Initialize the lookup values for the vert normals
	int itemCount = 0, itemBase = 0;
	for( int v=0; v<numVerts; v++ ) {
		if( (v&0xFF)==0 )
			vertBase[v>>8] = itemBase = itemCount;
		WORD offset = (WORD)(itemCount-itemBase);
		vertOffset[v] = offset;
		itemCount += grouping.GetGroupCount(v);
	}

	// 3. Initialize the lookup values for the face normals
	for( int f=0; f<numFaces; f++ ) {
		if( (f&0xFF)==0 )
			faceBase[f>>8] = itemBase = itemCount;
		WORD offset = (WORD)(itemCount-itemBase);
		if( grouping.GetBits(f) & VertFaceGrouping::BIT_ISFACETED ) {
			grouping.SetBits( f,  VertFaceGrouping::BIT_ISFACETED | offset );
			itemCount++;
		}
	}

	// 4. Delete the group count data from the grouping object (conserves memory)
	grouping.Compact();

	this->numItems = itemCount;
	valid = TRUE;
}

void VNormalChannel::Free() {
	grouping.Free();
	faceBase.SetCount(0);
	vertBase.SetCount(0);
	vertOffset.SetCount(0);
	tangentBasisSet.SetCount(0);
	tangentSignSet.SetCount(0);
	numItems = 0;
	valid = FALSE;
}

BOOL VNormalChannel::Valid() {return valid;}

void VNormalChannel::InitNormals( Mesh& mesh, Matrix3& tm, BOOL useMikkTangent ) {
	if (useMikkTangent) {
		return InitMikktNormals(mesh, tm);
	}

	Tab<Point3> normalsSet; //temp array

	// 1. Allocate the tangent array
	normalsSet.SetCount( numItems );
	if( numItems>0 ) memset( normalsSet.Addr(0), 0, numItems*sizeof(Point3) );

	// 2. Loop through the faces, calculating the tangent for each
	int numFaces = mesh.getNumFaces();
	for( int f=0; f<numFaces; f++ ) {
		Point3 norm = mesh.getFaceNormal(f);
		Face& geomFace = mesh.faces[f];

		// 3a. For smoothed faces,
		// add the normal to the sums for the three verts, and normalize below
		if( !IsFaceted(f) ) {
			for( int fv=0; fv<3; fv++ ) {
				int v = geomFace.v[fv];
				int index = FindIndex( f, v, fv );
				normalsSet[index] += norm;
			}
		}
		// 3b. For faceted faces, store one face normal
		else {
			int index = FindIndex( f, 0, 0 );
			normalsSet[index] = norm;
	}
}

	// 4. Normalize the normals
	for( int i=0; i<numItems; i++ ) {
		Point3& norm = normalsSet[i];
		norm = tm.VectorTransform( norm );
		norm.Unify();
}

	// 5. Finalize the operation
	memswp( &(this->normalsSet), &normalsSet, sizeof(normalsSet) );
	validNormals = TRUE;
}

void VNormalChannel::InitTangentBasis( Mesh& mesh, Matrix3& tm, int mapChannel, BOOL useMikkTangent ) {
	if (useMikkTangent) {
		return InitMikkTangentBasis(mesh, tm, mapChannel);
	}

	int numFaces = mesh.getNumFaces();
	Face   *geomFaces = mesh.faces;
	TVFace *mapFaces = mesh.mapFaces( mapChannel );
	Point3 *geomVerts = mesh.verts, *mapVerts = mesh.mapVerts( mapChannel );
	Point3  geomTri[3], mapTri[3], basisVec[2];
	Tab<TangentBasis> tangentBasisSet; //temp array

	//Error checking
	if( (mapFaces==NULL) || (mapVerts==NULL) ) {
		tangentBasisSet.SetCount(0);
		validTangentBasis = TRUE;
		return;
}

	// 1. Allocate the tangent array
	tangentBasisSet.SetCount( numItems );
	if( numItems>0 ) memset( tangentBasisSet.Addr(0), 0, numItems*sizeof(TangentBasis) );

	// 2. Loop through the faces, calculating the tangent for each
	for( int f=0; f<numFaces; f++ ) {
		Face& geomFace = geomFaces[f];
		geomTri[0] = geomVerts[ geomFace.v[0] ];
		geomTri[1] = geomVerts[ geomFace.v[1] ];
		geomTri[2] = geomVerts[ geomFace.v[2] ];

		TVFace& mapFace = mapFaces[f];
		mapTri[0] = mapVerts[ mapFace.t[0] ];
		mapTri[1] = mapVerts[ mapFace.t[1] ];
		mapTri[2] = mapVerts[ mapFace.t[2] ];

		ComputeTangentAndBinormal( mapTri, geomTri, basisVec );

		Point3 mapNormal = FNormalize( (mapTri[1] - mapTri[0]) ^ (mapTri[2] - mapTri[1]) );
		if( mapNormal.z<0 ) basisVec[1] = -basisVec[1]; //is the UV face flipped? flip the binormal

		// 3a. For smoothed faces,
		// add the tangent to the sums for the three verts, and normalize below
		if( !IsFaceted(f) ) {
			for( int fv=0; fv<3; fv++ ) {
				int v = geomFace.v[fv];
				int index = FindIndex( f, v, fv );
				tangentBasisSet[index].uBasis += basisVec[0];
				tangentBasisSet[index].vBasis += basisVec[1];
			}
		}
		// 3b. For faceted faces, store one face tangent
		else {
			int index = FindIndex( f, 0, 0 );
			tangentBasisSet[index].uBasis = basisVec[0];
			tangentBasisSet[index].vBasis = basisVec[1];
		}
}

	// 4. Normalize the tangents
	for( int i=0; i<numItems; i++ ) {
		TangentBasis& bv = tangentBasisSet[i];
		bv.uBasis = tm.VectorTransform( bv.uBasis );
		bv.vBasis = tm.VectorTransform( bv.vBasis );
		bv.uBasis.Unify();
		bv.vBasis.Unify();
	}

	// 5. Finalize the operation
	memswp( &(this->tangentBasisSet), &tangentBasisSet, sizeof(tangentBasisSet) );
	validTangentBasis = TRUE;
	}

// Utility to get Vertex Normal with specific face index and RVertex
static Point3 GetVertexNormal(Mesh* mesh, int faceNo, RVertex* rv)
{
	if (!rv) {
		return stubNormal;
	}
	Face* f = &mesh->faces[faceNo];
	DWORD smGroup = f->smGroup;
	int numNormals = (rv->rFlags & NORCT_MASK);
	Point3 vertexNormal;

	// Is normal specified
	// SPCIFIED is not currently used, but may be used in future versions.
	if (rv->rFlags & SPECIFIED_NORMAL) {
		vertexNormal = rv->rn.getNormal();
	}
	// If normal is not specified it's only available if the face belongs
	// to a smoothing group
	else if (((numNormals = rv->rFlags & NORCT_MASK) != 0) && smGroup) {
		// If there is only one vertex is found in the rn member.
		if (numNormals == 1) {
			vertexNormal = rv->rn.getNormal();
		}
		else {
			// If two or more vertices are there you need to step through them
			// and find the vertex with the same smoothing group as the current face.
			// You will find multiple normals in the ern member.
			for (int i = 0; i < numNormals; i++) {
				if (rv->ern[i].getSmGroup() & smGroup) {
					vertexNormal = rv->ern[i].getNormal();
				}
			}
		}
	}
	else {
		// Get the normal from the Face if no smoothing groups are there
		vertexNormal = mesh->getFaceNormal(faceNo);
	}

	return vertexNormal;
}

//===========================================================================
// MikkTSpace tangents computation for mesh with triangles.

class MikkTSpaceTriMesh : public ITangentsComputationCallback
{
public:
	MikkTSpaceTriMesh(Mesh* mesh, int mapChannel, Tab<TangentBasis>& tangentBasisSet, Tab<float>& tangentSignSet)
		: mesh(mesh), mapChannel(mapChannel), tangentBasisSet(tangentBasisSet), tangentSignSet(tangentSignSet) {
	}

	virtual int GetNumberOfFaces() const override
	{
		return mesh->getNumFaces();
	}
	virtual int GetNumberVerticesOfFace(const int faceIdx) const override
	{
		return 3;
	}
	virtual void GetVertex(Point3& position, const int faceIdx, const int vertIdx) override
	{
		const Face& geomFace = mesh->faces[faceIdx];
		const Point3& vertex = mesh->verts[geomFace.v[vertIdx]];
		for (int i = 0; i < 3; ++i) {
			position[i] = vertex[i];
		}
	}
	virtual void GetNormal(Point3& normal, const int faceIdx, const int vertIdx) override
	{
		Point3* pNormals = NULL;
		MeshNormalFace* pNf = NULL;
		MeshNormalSpec* mns = mesh->GetSpecifiedNormals();
		if (mns) {
			if (mns->GetNumNormals()) {
				pNormals = mns->GetNormalArray();
				pNf = mns->GetFaceArray();
			}
			else {
				mns = NULL;
			}
		}

		// Check specific normals
		if (pNf && pNormals) {
			const Point3& vnormal = pNormals[pNf[faceIdx].GetNormalID(vertIdx)];
			for (int i = 0; i < 3; ++i) {
				normal[i] = vnormal[i];
			}
		}
		else {
			const Face& geomFace = mesh->faces[faceIdx];
			const Point3& vnormal = GetVertexNormal(mesh, faceIdx, mesh->getRVertPtr(geomFace.v[vertIdx]));
			for (int i = 0; i < 3; ++i) {
				normal[i] = vnormal[i];
			}
		}
	}

	virtual void GetTexture(Point2& texture, const int faceIdx, const int vertIdx) override
	{
		TVFace& mapFace = mesh->mapFaces(mapChannel)[faceIdx];
		const Point3& textureCoord = mesh->mapVerts(mapChannel)[mapFace.t[vertIdx]];
		for (int i = 0; i < 2; ++i) {
			texture[i] = textureCoord[i];
		}
	}
	virtual void SetTangent(const Point3& tangent, const Point3& binormal, const float handedness, const int faceIdx, const int vertIdx) override
	{
		// Set tangent
		Point3 &vertexTangent = tangentBasisSet[faceIdx * 3 + vertIdx].uBasis;
		for (int i = 0; i < 3; ++i) {
			vertexTangent[i] = tangent[i];
		}

		// Set binormal
		Point3 &vertexBinormal = tangentBasisSet[faceIdx * 3 + vertIdx].vBasis;
		for (int i = 0; i < 3; ++i) {
			vertexBinormal[i] = binormal[i];
		}

		// Set tangent sign, could be used to calculate binormal/bitangent later in pixel level
		tangentSignSet[faceIdx * 3 + vertIdx] = handedness;
	}


	// Input data
	Mesh* mesh{ nullptr };
	int mapChannel{ 0 };

	// Output tangent/binormal data
	Tab<TangentBasis>& tangentBasisSet;
	Tab<float>& tangentSignSet;
};
// End of MikkTSpace tangents computation for mesh with triangles.
//===========================================================================

void VNormalChannel::InitMikktNormals(Mesh& mesh, Matrix3& tm) {
	int numFaces = mesh.getNumFaces();

	//Error checking
	if (numFaces == 0) {
		validNormals = TRUE;
		return;
	}

	Tab<Point3> normalsSet; //temp array
	int numVertices = numFaces * 3;
	// 1. Allocate the tangent array
	normalsSet.SetCount(numVertices);
	if (numVertices > 0) memset(normalsSet.Addr(0), 0, numVertices * sizeof(Point3));

	Point3* pNormals = NULL;
	MeshNormalFace* pNf = NULL;
	MeshNormalSpec* mns = mesh.GetSpecifiedNormals();
	if (mns) {
		if (mns->GetNumNormals()) {
			pNormals = mns->GetNormalArray();
			pNf = mns->GetFaceArray();
		}
		else {
			mns = NULL;
		}
	}

	// 2. Loop through the faces, calculating the normals for vertices of each
	for (int f = 0; f < numFaces; f++) {
		// Check specific normals
		if (pNf && pNormals) {
			for (int i = 0; i < 3; ++i) {
				normalsSet[f*3 + i] = pNormals[pNf[f].GetNormalID(i)];
			}
			
		}
		else {
			const Face& geomFace = mesh.faces[f];
			for (int i = 0; i < 3; ++i) {
				normalsSet[f * 3 + i] = GetVertexNormal(&mesh, f, mesh.getRVertPtr(geomFace.v[i]));
			}
		}
	}

	// 3. Transform the normals to camera space and normalize
	for (int i = 0; i < numVertices; i++) {
		Point3& norm = normalsSet[i];
		norm = tm.VectorTransform(norm);
		norm.Unify();
	}

	// 5. Finalize the operation
	memswp(&(this->normalsSet), &normalsSet, sizeof(normalsSet));
	validNormals = TRUE;
}

void VNormalChannel::InitMikkTangentBasis(Mesh& mesh, Matrix3& tm, int mapChannel) {
	int numFaces = mesh.getNumFaces();
	TVFace *mapFaces = mesh.mapFaces(mapChannel);
	Point3 *mapVerts = mesh.mapVerts(mapChannel);

	//Error checking
	if ((mapFaces == NULL) || (mapVerts == NULL) || (numFaces==0)) {
		validTangentBasis = TRUE;
		return;
	}

	// 1. Allocate the tangent array
	Tab<TangentBasis> tangentBasisSet; //temp array
	int numVertices = numFaces * 3;
	tangentBasisSet.SetCount(numVertices);
	memset(tangentBasisSet.Addr(0), 0, numVertices * sizeof(TangentBasis));
	tangentSignSet.SetCount(numVertices);
	memset(tangentBasisSet.Addr(0), 0, numVertices * sizeof(float));

	// 2. Compute tagents
	MikkTSpaceTriMesh cb(&mesh, mapChannel, tangentBasisSet, tangentSignSet);
	ComputeMikkTangents(&cb);

	// 3. Transform the tangents to camera space and normalize
	for (int i = 0; i < numVertices; i++) {
		TangentBasis& bv = tangentBasisSet[i];
		bv.uBasis = tm.VectorTransform(bv.uBasis);
		bv.vBasis = tm.VectorTransform(bv.vBasis);
		bv.uBasis.Unify();
		bv.vBasis.Unify();
	}

	// 4. Finalize the operation
	memswp(&(this->tangentBasisSet), &tangentBasisSet, sizeof(tangentBasisSet));
	validTangentBasis = TRUE;
}

BOOL VNormalChannel::ValidTangentBasis() {
	return validTangentBasis;
}

BOOL VNormalChannel::ValidNormals() {
	return validNormals;
}

Point3& VNormalChannel::GetNormal( int faceIndex, int vertIndex, int faceVertIndex, BOOL useMikkTangent ) {
	if (useMikkTangent) {
		return GetMikktNormal(faceIndex, vertIndex, faceVertIndex);
	}
	int index = FindIndex( faceIndex, vertIndex, faceVertIndex );
	if( index>=normalsSet.Count() ) return stubNormal;
	return normalsSet[index];
}

Point3& VNormalChannel::GetMikktNormal(int faceIndex, int vertIndex, int faceVertIndex) {
	int index = faceIndex * 3 + faceVertIndex;
	if (index >= normalsSet.Count()) return stubNormal;
	return normalsSet[index];
}

const float& VNormalChannel::GetTangentSign(int faceIndex, int vertIndex, int faceVertIndex) {
	int index = faceIndex * 3 + faceVertIndex;
	if (index >= tangentSignSet.Count()) return stubTangentSign;
	return tangentSignSet[index];
}

TangentBasis& VNormalChannel::GetTangentBasis( int faceIndex, int vertIndex, int faceVertIndex, BOOL useMikkTangent ) {
	if (useMikkTangent) {
		return GetMikkTangentBasis(faceIndex, vertIndex, faceVertIndex);
	}
	int index = FindIndex( faceIndex, vertIndex, faceVertIndex );
	if( index>=tangentBasisSet.Count() ) return stubTangentBasis;
	return tangentBasisSet[index];
}

TangentBasis& VNormalChannel::GetMikkTangentBasis(int faceIndex, int vertIndex, int faceVertIndex) {
	int index = faceIndex * 3 + faceVertIndex;
	if (index >= tangentBasisSet.Count()) return stubTangentBasis;
	return tangentBasisSet[index];
}

BOOL VNormalChannel::IsFaceted( int faceIndex ) {
	WORD faceBits = grouping.GetBits(faceIndex);
	return (faceBits & VertFaceGrouping::BIT_ISFACETED?  TRUE:FALSE);
}

int VNormalChannel::FindIndex( int faceIndex, int vertIndex, int faceVertIndex ) {
	int index;
	WORD faceBits = grouping.GetBits( faceIndex );
	if( !IsFaceted(faceIndex) ) {
		int group = grouping.GetGroup( faceIndex, faceVertIndex );
		index = vertBase[vertIndex>>8] + vertOffset[vertIndex] + group;
	}
	else
		index = faceBase[faceIndex>>8] + (faceBits&0x7FFF);
	return index;
}



//===========================================================================
//
// Class VNormalBank
//
// Holds one VNormalChannel for each mapping channel of an object
//
//===========================================================================

VNormalBank::VNormalBank() { }

void VNormalBank::Free() {
	int count = channels.Count();
	for( int i=0; i<count; i++ )
		if( channels[i]!=NULL ) delete channels[i];
	channels.SetCount(0);
}

void VNormalBank::InitChannel( Mesh& mesh, int mapChannel ) {
	int m = mapChannel + NUM_HIDDENMAPS;
	VNormalChannel* null = NULL;

	int numAppend = ((m+1) - channels.Count());
	for( int i=0; i<numAppend; i++ )
		channels.Append( 1, &null );

	VNormalChannel* channel = channels[m];
	if( channel==NULL )
		channel = new VNormalChannel;

	if( !channel->Valid() )
		channel->Init( mesh, mapChannel );

	channels[m] = channel; //Finalize the operation
	}
		
BOOL VNormalBank::ValidChannel( int mapChannel ) {
	VNormalChannel* channel = GetChannel(mapChannel);
	return ((channel!=NULL) && channel->Valid()?  TRUE:FALSE);
	}

VNormalChannel* VNormalBank::GetChannel( int mapChannel ) {
	int m = mapChannel + NUM_HIDDENMAPS;
	if( m>=channels.Count() ) return NULL;
	return channels[m];
}



//===========================================================================
//
// Class VNormalMgr
//
// Holds a normal vector and related data at each vertex & smoothing group;
// that is, one piece of data per vertex per smoothing group at the vertex.
//
//===========================================================================

VNormalMgr theVNormalMgr;

VNormalMgr::VNormalMgr()
{
}

VNormalMgr::~VNormalMgr() {
	Free();
}

void VNormalMgr::Free() {
	lock.EnterWrite();
	for( int i=0; i<banks.Count(); i++ )
		if( banks[i]!=NULL ) delete banks[i];
	banks.SetCount(0);
	lock.ExitWrite();
}

void VNormalMgr::InitChannel( Mesh& mesh, int nodeID, int mapChannel ) {
	lock.EnterWrite();
	InitChannel_Internal( mesh, nodeID, mapChannel );
	lock.ExitWrite();
}

void VNormalMgr::InitChannel_Internal( Mesh& mesh, int nodeID, int mapChannel ) {
	if( ValidChannel_Internal(nodeID,mapChannel) ) { // initialized while waiting on the semaphore?
		return;
	}

	VNormalBank* null = NULL;
	int numAppend = ((nodeID+1) - banks.Count());
	for( int i=0; i<numAppend; i++ )
		banks.Append( 1, &null );

	if( banks[nodeID]==NULL ) {
		banks[nodeID] = new VNormalBank;
		banks[nodeID]->Init();
	}
	banks[nodeID]->InitChannel( mesh, mapChannel );
}

BOOL VNormalMgr::ValidChannel( int nodeID, int mapChannel ) {
	return (GetChannel(nodeID,mapChannel)==NULL?  FALSE:TRUE);
}

BOOL VNormalMgr::ValidChannel_Internal( int nodeID, int mapChannel ) {
	return (GetChannel_Internal(nodeID,mapChannel)==NULL?  FALSE:TRUE);
}

VNormalChannel* VNormalMgr::GetChannel( int nodeID, int mapChannel ) {
	lock.EnterRead();
	VNormalChannel* retval = GetChannel_Internal(nodeID,mapChannel);
	lock.ExitRead();
	return retval;
}

VNormalChannel* VNormalMgr::GetChannel_Internal( int nodeID, int mapChannel ) {
	if( banks.Count()<=nodeID ) return FALSE;

	VNormalBank* bank = banks[nodeID];
	if( bank==NULL ) return NULL;

	return bank->GetChannel( mapChannel );
	}

//-- Convenience methods

void VNormalMgr::InitTangentBasis( ShadeContext& sc, int mapChannel ) {
	int nodeID = sc.NodeID();
	RenderInstance* inst = sc.globContext->GetRenderInstance(nodeID);
	if( (inst==NULL) || (inst->mesh==NULL) ) return;

	lock.EnterWrite();

	// build the normals if not available
	inst->mesh->checkNormals( TRUE );

	MaxSDK::Graphics::INormalMappingManager* nmMgr = static_cast<MaxSDK::Graphics::INormalMappingManager*>(GetCOREInterface(NORMALMAPPINGMGR_INTERFACE));
	useMikkTangent = nmMgr->GetTangentBasisMode() == MaxSDK::Graphics::kMikkT ? TRUE : FALSE;

	InitChannel_Internal( *(inst->mesh), nodeID, mapChannel);
	VNormalChannel* channel = GetChannel_Internal( nodeID, mapChannel );
	if( !channel->ValidTangentBasis() ) // initialized while waiting on the semaphore?
		 channel->InitTangentBasis( *(inst->mesh), inst->objToCam, mapChannel, useMikkTangent);

	// MIKKT
	if (!channel->ValidNormals())
		channel->InitNormals(*(inst->mesh), inst->normalObjToCam, useMikkTangent);

	lock.ExitWrite();
}

void VNormalMgr::GetTangentBasis( ShadeContext& sc, int mapChannel, TangentBasis& tangentBasis ) {
	DbgAssert( sc.globContext!=NULL );
	int nodeID = sc.NodeID();
	RenderInstance* inst = sc.globContext->GetRenderInstance(nodeID);
	if( (inst==NULL) || 
		 (inst->mesh==NULL) ||
		 NULL == inst->mesh->faces ||
		 inst->mesh->getNumFaces() <= sc.FaceNumber() )
	{
		return;
	} 

	lock.EnterRead();

	VNormalChannel* channel = GetChannel_Internal( nodeID, mapChannel );
	while( (channel==NULL) || (!channel->ValidTangentBasis()) ) {
		lock.ExitRead(); //release lock while initializing
		InitTangentBasis( sc, mapChannel );
		lock.EnterRead(); //regain lock after initializing
		channel = GetChannel_Internal( nodeID, mapChannel );
	}

	int faceIndex = sc.FaceNumber();
	Face& f = inst->mesh->faces[faceIndex];
	// MIKKT
	// Do not normalize the interpolated tangents/bitangents for Mikktspace
	if (useMikkTangent) {
		DWORD *v = f.v;
		Point3 triBary = sc.BarycentricCoords();
		TangentBasis& b0 = channel->GetTangentBasis(faceIndex, v[0], 0, true);
		TangentBasis& b1 = channel->GetTangentBasis(faceIndex, v[1], 1, true);
		TangentBasis& b2 = channel->GetTangentBasis(faceIndex, v[2], 2, true);

		Point3 normal = stubNormal;
		Point3& n0 = channel->GetNormal(faceIndex, v[0], 0, true);
		Point3& n1 = channel->GetNormal(faceIndex, v[1], 1, true);
		Point3& n2 = channel->GetNormal(faceIndex, v[2], 2, true);

		// Do not normalize the interpolated normals for Mikktspace
		normal = ((triBary.x*n0) + (triBary.y*n1) + (triBary.z*n2));
		tangentBasis.nBasis = normal;

		bool calculateBitangentPerPixel = static_cast<MaxSDK::Graphics::INormalMappingManager*>(GetCOREInterface(NORMALMAPPINGMGR_INTERFACE))->GetCalculateBitangentPerPixel();
		if (normal != stubNormal) {
			if (calculateBitangentPerPixel) {
				tangentBasis.uBasis = ((triBary.x*b0.uBasis) + (triBary.y*b1.uBasis) + (triBary.z*b2.uBasis));
				// Calculate the binormal/bitangent to make sure it's orthogonal
				const float& tangentSign0 = channel->GetTangentSign(faceIndex, v[0], 0);
				const float& tangentSign1 = channel->GetTangentSign(faceIndex, v[1], 1);
				const float& tangentSign2 = channel->GetTangentSign(faceIndex, v[2], 2);
				float tangentSign = (tangentSign0 * triBary.x + tangentSign1 * triBary.y + tangentSign2 * triBary.z) < 0 ? (-1.0f) : 1.0f;
				tangentBasis.vBasis = tangentSign * CrossProd(normal, tangentBasis.uBasis);
			}
			else {
				tangentBasis.uBasis = ((triBary.x*b0.uBasis) + (triBary.y*b1.uBasis) + (triBary.z*b2.uBasis));
				tangentBasis.vBasis = ((triBary.x*b0.vBasis) + (triBary.y*b1.vBasis) + (triBary.z*b2.vBasis));
			}
		}
		else {
			tangentBasis = stubTangentBasis;
			DbgAssert(FALSE);
		}
	}
	else {
		DWORD smGroup = f.smGroup;
		if (smGroup == 0)
			tangentBasis = channel->GetTangentBasis(faceIndex, 0, 0, false);
		else {
			DWORD *v = f.v;
			TangentBasis& b0 = channel->GetTangentBasis(faceIndex, v[0], 0, false); //returned in camera space
			TangentBasis& b1 = channel->GetTangentBasis(faceIndex, v[1], 1, false); //returned in camera space
			TangentBasis& b2 = channel->GetTangentBasis(faceIndex, v[2], 2, false); //returned in camera space

			Point3 bary = sc.BarycentricCoords();
			tangentBasis.uBasis = ((bary.x*b0.uBasis) + (bary.y*b1.uBasis) + (bary.z*b2.uBasis)).Normalize();
			tangentBasis.vBasis = ((bary.x*b0.vBasis) + (bary.y*b1.vBasis) + (bary.z*b2.vBasis)).Normalize();
		}
	}

	lock.ExitRead();
}

VNormalMgr* GetVNormalMgr() {
	return &theVNormalMgr;
}
