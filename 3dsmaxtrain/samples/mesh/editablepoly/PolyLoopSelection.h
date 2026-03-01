#pragma once

//*****************************************************************************
// Copyright 2020 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise accompanies
// this software in either electronic or hard copy form.
//
//*****************************************************************************
//
//	FILE: PolyLoopSelection.h
//  DESCRIPTION: Utilities to perform loop selection on an MNMesh
//	CREATED BY:	Nicholas Frechette
//*****************************************************************************

#include <Geom/bitarray.h>
#include <mnmesh.h>

inline bool AreVerticesAdjacent(MNMesh& mesh, int vertexIndex0, int vertexIndex1)
{
	if (vertexIndex0 < 0 || vertexIndex1 < 0)
		return false; // Index invalid

	if (vertexIndex0 == vertexIndex1)
		return false; // Face is the same, not adjacent

	BitArray vertices;
	vertices.SetSize(mesh.numv);
	vertices.Set(vertexIndex0);

	MNMeshSelectionConverter converter;

	BitArray edges;
	converter.VertexToEdge(mesh, vertices, edges);

	for (int edgeIndex = 0; edgeIndex < mesh.nume; ++edgeIndex)
	{
		if (!edges[edgeIndex])
			continue; // Edge isn't connected to our first vertex

		const MNEdge& edge = mesh.e[edgeIndex];
		if (edge.GetFlag(MN_DEAD))
			continue; // Skip dead edges

		if (edge.v1 == vertexIndex1 || edge.v2 == vertexIndex1)
			return true; // Found an edge connecting both vertices
	}

	// No edge found connecting both vertices
	return false;
}

inline bool AreFacesAdjacent(MNMesh& mesh, int faceIndex0, int faceIndex1)
{
	if (faceIndex0 < 0 || faceIndex1 < 0)
		return false; // Index invalid

	if (faceIndex0 == faceIndex1)
		return false; // Face is the same, not adjacent

	const MNFace& face0 = mesh.f[faceIndex0];
	const MNFace& face1 = mesh.f[faceIndex1];
	if (face0.GetFlag(MN_DEAD) || face1.GetFlag(MN_DEAD))
		return false; // Face is dead

	for (int e = 0; e < face0.deg; ++e)
	{
		const int edgeIndex = face0.edg[e];
		MNEdge& edge = mesh.e[edgeIndex];
		const int otherFace = edge.OtherFace(faceIndex0);
		if (otherFace == faceIndex1)
			return true;
	}

	// Checked every edge and we didn't find a face that matches our other index
	return false;
}

inline bool AreEdgesAdjacent(MNMesh& mesh, int edgeIndex0, int edgeIndex1, bool& outIsRing)
{
	if (edgeIndex0 < 0 || edgeIndex1 < 0)
		return false; // Index invalid

	if (edgeIndex0 == edgeIndex1)
		return false; // Edge is the same, not adjacent

	const MNEdge& edge0 = mesh.e[edgeIndex0];
	const MNEdge& edge1 = mesh.e[edgeIndex1];
	if (edge0.GetFlag(MN_DEAD) || edge1.GetFlag(MN_DEAD))
		return false; // Edge is dead

	// first check for the loop
	const int v[2] = { edge0.v1, edge0.v2 };
	for (int i = 0; i < 2; i++)
	{
		const int vertexIndex = v[i];
		mesh.OrderVert(vertexIndex);

		const Tab<int>& vertexEdges = mesh.vedg[vertexIndex];
		if (vertexEdges.Count() != 4)
			continue; // Face must be a quad

		for (int j = 0; j < 4; j++)
		{
			if (vertexEdges[j] != edgeIndex0)
				continue; // Not our edge

			int oppoEdge = (j + 2) % 4;
			int oppoEdgeIndex = vertexEdges[oppoEdge];
			if (oppoEdgeIndex == edgeIndex1)
			{
				outIsRing = false;
				return true; // Edges are adjacent on a loop
			}
		}
	}

	// now check for the ring
	const int f[2] = { edge0.f1, edge0.f2 };
	for (int i = 0; i < 2; i++)
	{
		const int faceIndex = f[i];

		if (faceIndex < 0)
			continue; // Open edge

		const MNFace& f = mesh.f[faceIndex];

		if (f.deg != 4)
			continue; // Face must be a quad

		// find the our edge on the face
		for (int j = 0; j < 4; j++)
		{
			if (f.edg[j] != edgeIndex0)
				continue; // Not our edge

			// get the opposite edge
			int oppoEdge = (j + 2) % 4;
			int oppoEdgeIndex = f.edg[oppoEdge];
			if (oppoEdgeIndex == edgeIndex1)
			{
				outIsRing = true;
				return true; // Edges are adjacent on a ring
			}
		}
	}

	// No viable candidate found
	return false;
}

inline int FindVertexLoopPath(MNMesh& mesh, BitArray& loop, int startVertex, int endVertex, BitArray& outPathVertices)
{
	// TODO: If we ever need to optimize this, we can traverse both paths in parallel or we can do them in lock step
	// within a single loop to early out as soon as we find a valid path

	outPathVertices.SetSize(mesh.numv);
	outPathVertices.ClearAll();

	if (!loop[startVertex] || !loop[endVertex])
		return 0; // Start or end vertex isn't on the loop, no path is possible

	BitArray verticesVisited;
	verticesVisited.SetSize(mesh.numv);

	int pathLen = 0;
	bool firstPass = true;

	MNMeshSelectionConverter converter;

	// Find the edges that our loop references
	BitArray edges;
	converter.VertexToEdge(mesh, loop, edges);

	{
		int pathVertexIndex = startVertex;
		while (!verticesVisited[pathVertexIndex])
		{
			if (pathVertexIndex == endVertex)
				break; // Found out what we were looking for

			verticesVisited.Set(pathVertexIndex);
			outPathVertices.Set(pathVertexIndex);
			pathLen++;

			const int prevPathVertexIndex = pathVertexIndex;
			for (int edgeIndex = 0; edgeIndex < mesh.nume; ++edgeIndex)
			{
				if (!edges[edgeIndex])
					continue; // This edge isn't part of our loop

				MNEdge& edge = mesh.e[edgeIndex];
				if (pathVertexIndex != edge.v1 && pathVertexIndex != edge.v2)
					continue; // This edge doesn't contain our current vertex

				const int otherVert = edge.OtherVert(pathVertexIndex);

				if (verticesVisited[otherVert])
					continue; // Already visited this vertex

				if (!loop[otherVert])
					continue; // Vertex isn't in our loop

				// Found a new vertex we haven't visited that is in our loop, traverse it
				pathVertexIndex = otherVert;
				break;
			}

			if (prevPathVertexIndex == pathVertexIndex)
			{
				// Our vertex index didn't change, we couldn't find a new vertex
				if (firstPass)
				{
					// We failed to find our end vertex and ran out of loop vertices, try the other way around
					verticesVisited.Clear(startVertex);
					pathVertexIndex = startVertex;
					outPathVertices.ClearAll();
					pathLen = 0;
					firstPass = false;
				}
				else
				{
					// We failed to find our end vertex on the loop from our start position, something went terribly
					// wrong
					outPathVertices.ClearAll();
					pathLen = 0;
					break;
				}
			}
		}
	}

	if (firstPass)
	{
		// We found a valid path but might not be the shortest if we didn't explore the other way around, do so now

		// Reset our starting point so we can try again
		verticesVisited.Clear(startVertex);

		BitArray otherPathVertices;
		otherPathVertices.SetSize(mesh.numf);

		int otherPathLen = 0;

		int pathVertexIndex = startVertex;
		while (!verticesVisited[pathVertexIndex])
		{
			if (pathVertexIndex == endVertex)
				break; // Found out what we were looking for

			verticesVisited.Set(pathVertexIndex);
			otherPathVertices.Set(pathVertexIndex);
			otherPathLen++;

			const int prevPathVertexIndex = pathVertexIndex;
			for (int edgeIndex = 0; edgeIndex < mesh.nume; ++edgeIndex)
			{
				if (!edges[edgeIndex])
					continue; // This edge isn't part of our loop

				MNEdge& edge = mesh.e[edgeIndex];
				if (pathVertexIndex != edge.v1 && pathVertexIndex != edge.v2)
					continue; // This edge doesn't contain our current vertex

				const int otherVert = edge.OtherVert(pathVertexIndex);

				if (verticesVisited[otherVert])
					continue; // Already visited this vertex

				if (!loop[otherVert])
					continue; // Vertex isn't in our loop

				// Found a new vertex we haven't visited that is in our loop, traverse it
				pathVertexIndex = otherVert;
				break;
			}

			if (prevPathVertexIndex == pathVertexIndex)
			{
				// Our vertex index didn't change, we couldn't find a new vertex
				// We failed to find our end vertex on the loop from our start position, use the first path we found
				otherPathLen = pathLen + 1; // Force to be longer so we don't take it
				break;
			}
		}

		if (otherPathLen < pathLen)
		{
			// Our new path is shorter, use it
			pathLen = otherPathLen;
			outPathVertices = otherPathVertices;
		}
	}

	return pathLen;
}

// returns the shortest loop in selection loop that begins at startFace and ends at endFace
// loop holds the selection of the loop faces out of startFace but if the mnmesh is irregular
// the loops may intersect other loops and share faces so need to be careful with it.
// outPathFaces is the shortes loop connected between startFace and endFace
// returns the length of the path
namespace
{
	//this represents a loop path coming out of a faces edge	
	class Path
	{
		public:
			bool mDeadEnd = false;
			std::vector<int> mPath;
			std::vector<int> mEdges;

	};
} // namespace

inline int FindFaceLoopPath(const MNMesh& mesh, const BitArray& loop, int startFace, int endFace, BitArray& outPathFaces)
{
	outPathFaces.SetSize(mesh.numf);
	outPathFaces.ClearAll();

	if (!loop[startFace] || !loop[endFace])
		return 0; // Start or end face isn't on the loop, no path is possible
	//make sure the faces actually are valid
	if (mesh.f[startFace].GetFlag(MN_DEAD))
		return 0;
	if (mesh.f[endFace].GetFlag(MN_DEAD))
		return 0;
	//if the start or end face are not degree 4 bail
	if (mesh.f[startFace].deg != 4)
		return 0;
	if (mesh.f[endFace].deg != 4)
		return 0;
	//vector of all the loop paths coming out of the start face
	std::vector<Path> paths;
	size_t deg = mesh.f[startFace].deg;
	paths.resize(deg);
	//load up the initial start of the loop paths from our start face
	for (size_t i = 0; i < deg; ++i)
	{
		paths[i].mPath.push_back(startFace);
		int edgeIndex = mesh.f[startFace].edg[i];
		paths[i].mEdges.push_back(edgeIndex);
	}
	int pathLen = 0;
	bool done = false;
	size_t level = 0;
	//we seacrhing the loops until all are dead in case we did not find one or one hits the endFace
	while (!done)
	{
		//advance each loop
		bool continueLooping = false;
		++level;
		//loop thru each potential loop
		for (size_t i = 0; i < deg; ++i)
		{
			//skip paths that terminated			
			if (!paths[i].mDeadEnd)
			{
				//pop the last face and edge off the path
				int pathIndex = int(paths[i].mPath.size() - 1);
				int faceIndex = paths[i].mPath[pathIndex];
				int edgeIndex = paths[i].mEdges[pathIndex];
				DbgAssert(paths[i].mPath.size() == paths[i].mEdges.size());

				//now compute the connected loop face from the last face
				int opposingFace = -1;
				int opposingEdge = -1;

				
				const MNEdge& testEdge = mesh.e[edgeIndex];

				opposingFace = testEdge.f1;
				if (opposingFace == faceIndex)
				{
					opposingFace = testEdge.f2;
				}

				
				if (opposingFace != -1) //make sure we are not an open edge
				{
					const MNFace& testFace = mesh.f[opposingFace];
					int ithIndex = testFace.EdgeIndex(edgeIndex);
					ithIndex = (ithIndex + (testFace.deg / 2)) % testFace.deg;
					opposingEdge = testFace.edg[ithIndex];
				}

				// if we hit the end face we are done and return this loop
				if (opposingFace == endFace)
				{
					//hit end face first terminate
					paths[i].mPath.push_back(opposingFace);
					paths[i].mEdges.push_back(opposingEdge);
					done = true;
					
					for (int j = 0; j < paths[i].mPath.size(); ++j)
					{
						outPathFaces.Set(paths[i].mPath[j]);
					}
					return int(paths[i].mPath.size());
				}
				else if (opposingFace == -1) // hit an open face before the end face this is a dead loop
				{
					paths[i].mDeadEnd = true;
				}
				else if (!loop[opposingFace]) //not in our filter face selection we stop for this loop
				{
					paths[i].mDeadEnd = true;
				}
				else if (opposingFace == startFace) // looped back to start before hitting end this is a dead loop
				{
					paths[i].mDeadEnd = true;
				}
				else 
				{
					const MNFace& f = mesh.f[opposingFace];
					if (f.deg != 4) //stop at non quads since they stop loops
					{
						//we hit a non quad which stops a loop so this is a dead end
						paths[i].mPath.push_back(opposingFace);
						paths[i].mEdges.push_back(opposingEdge);
						paths[i].mDeadEnd = true;
					}
					else //we can continue advancing down the loop to the next face push the new face and edge onto the loop
					{
						paths[i].mPath.push_back(opposingFace);
						paths[i].mEdges.push_back(opposingEdge);
						continueLooping = true;
					}
				}
			}
		}
		//all loops are dead we can stop
		if (continueLooping == false)
			done = true;
	}
	return pathLen;
}

inline int FindEdgeLoopPathThroughFaces(MNMesh& mesh, const BitArray& loop, int startEdge, int endEdge, BitArray& outPathEdges)
{
	// TODO: If we ever need to optimize this, we can traverse both paths in parallel or we can do them in lock step
	// within a single loop to early out as soon as we find a valid path

	outPathEdges.SetSize(mesh.nume);
	outPathEdges.ClearAll();

	if (!loop[startEdge] || !loop[endEdge])
		return 0; // Start or end edge isn't on the loop, no path is possible

	BitArray edgesVisited;
	edgesVisited.SetSize(mesh.nume);

	int pathLen = 0;
	bool firstPass = true;

	{
		int pathEdgeIndex = startEdge;
		while (!edgesVisited[pathEdgeIndex])
		{
			if (pathEdgeIndex == endEdge)
				break; // Found out what we were looking for

			edgesVisited.Set(pathEdgeIndex);
			outPathEdges.Set(pathEdgeIndex);
			pathLen++;

			const int prevPathEdgeIndex = pathEdgeIndex;

			MNEdge& edge = mesh.e[pathEdgeIndex];
			if (edge.f1 >= 0)
			{
				// Try to find our next edge on the first face
				MNFace& face = mesh.f[edge.f1];
				for (int faceEdgeIndex = 0; faceEdgeIndex < face.deg; ++faceEdgeIndex)
				{
					const int newEdgeIndex = face.edg[faceEdgeIndex];

					if (edgesVisited[newEdgeIndex])
						continue; // Already visited this edge

					if (!loop[newEdgeIndex])
						continue; // Edge isn't in our loop

					// Found a new edge we haven't visited that is in our loop, traverse it
					pathEdgeIndex = newEdgeIndex;
					break;
				}
			}

			if (prevPathEdgeIndex == pathEdgeIndex && edge.f2 >= 0)
			{
				// Try to find our next edge on the second face
				MNFace& face = mesh.f[edge.f2];
				for (int faceEdgeIndex = 0; faceEdgeIndex < face.deg; ++faceEdgeIndex)
				{
					const int newEdgeIndex = face.edg[faceEdgeIndex];

					if (edgesVisited[newEdgeIndex])
						continue; // Already visited this edge

					if (!loop[newEdgeIndex])
						continue; // Edge isn't in our loop

					// Found a new edge we haven't visited that is in our loop, traverse it
					pathEdgeIndex = newEdgeIndex;
					break;
				}
			}

			if (prevPathEdgeIndex == pathEdgeIndex)
			{
				// Our edge index didn't change, we couldn't find a new edge
				if (firstPass)
				{
					// We failed to find our end edge and ran out of loop edges, try the other way around
					edgesVisited.Clear(startEdge);
					pathEdgeIndex = startEdge;
					outPathEdges.ClearAll();
					pathLen = 0;
					firstPass = false;
				}
				else
				{
					// We failed to find our end edge on the loop from our start position, something went terribly wrong
					outPathEdges.ClearAll();
					pathLen = 0;
					break;
				}
			}
		}
	}

	if (firstPass)
	{
		// We found a valid path but might not be the shortest if we didn't explore the other way around, do so now

		// Reset our starting point so we can try again
		edgesVisited.Clear(startEdge);

		BitArray otherPathEdges;
		otherPathEdges.SetSize(mesh.nume);

		int otherPathLen = 0;

		int pathEdgeIndex = startEdge;
		while (!edgesVisited[pathEdgeIndex])
		{
			if (pathEdgeIndex == endEdge)
				break; // Found out what we were looking for

			edgesVisited.Set(pathEdgeIndex);
			otherPathEdges.Set(pathEdgeIndex);
			otherPathLen++;

			const int prevPathEdgeIndex = pathEdgeIndex;

			MNEdge& edge = mesh.e[pathEdgeIndex];
			if (edge.f1 >= 0)
			{
				// Try to find our next edge on the first face
				MNFace& face = mesh.f[edge.f1];
				for (int faceEdgeIndex = 0; faceEdgeIndex < face.deg; ++faceEdgeIndex)
				{
					const int newEdgeIndex = face.edg[faceEdgeIndex];

					if (edgesVisited[newEdgeIndex])
						continue; // Already visited this edge

					if (!loop[newEdgeIndex])
						continue; // Edge isn't in our loop

					// Found a new edge we haven't visited that is in our loop, traverse it
					pathEdgeIndex = newEdgeIndex;
					break;
				}
			}

			if (prevPathEdgeIndex == pathEdgeIndex && edge.f2 >= 0)
			{
				// Try to find our next edge on the second face
				MNFace& face = mesh.f[edge.f2];
				for (int faceEdgeIndex = 0; faceEdgeIndex < face.deg; ++faceEdgeIndex)
				{
					const int newEdgeIndex = face.edg[faceEdgeIndex];

					if (edgesVisited[newEdgeIndex])
						continue; // Already visited this edge

					if (!loop[newEdgeIndex])
						continue; // Edge isn't in our loop

					// Found a new edge we haven't visited that is in our loop, traverse it
					pathEdgeIndex = newEdgeIndex;
					break;
				}
			}

			if (prevPathEdgeIndex == pathEdgeIndex)
			{
				// Our vertex index didn't change, we couldn't find a new vertex
				// We failed to find our end vertex on the loop from our start position, use the first path we found
				otherPathLen = pathLen + 1; // Force to be longer so we don't take it
				break;
			}
		}

		if (otherPathLen < pathLen)
		{
			// Our new path is shorter, use it
			pathLen = otherPathLen;
			outPathEdges = otherPathEdges;
		}
	}

	return pathLen;
}

inline int FindEdgeLoopPathThroughVertices(MNMesh& mesh, const BitArray& loop, int startEdge, int endEdge, BitArray& outPathEdges)
{
	// TODO: If we ever need to optimize this, we can traverse both paths in parallel or we can do them in lock step
	// within a single loop to early out as soon as we find a valid path

	outPathEdges.SetSize(mesh.nume);
	outPathEdges.ClearAll();

	if (!loop[startEdge] || !loop[endEdge])
		return 0; // Start or end edge isn't on the loop, no path is possible

	BitArray edgesVisited;
	edgesVisited.SetSize(mesh.nume);

	int pathLen = 0;
	bool firstPass = true;

	{
		int pathEdgeIndex = startEdge;
		while (!edgesVisited[pathEdgeIndex])
		{
			if (pathEdgeIndex == endEdge)
				break; // Found out what we were looking for

			edgesVisited.Set(pathEdgeIndex);
			outPathEdges.Set(pathEdgeIndex);
			pathLen++;

			const int prevPathEdgeIndex = pathEdgeIndex;

			const MNEdge& edge = mesh.e[pathEdgeIndex];
			for (int edgeIndex = 0; edgeIndex < mesh.nume; ++edgeIndex)
			{
				if (edgesVisited[edgeIndex])
					continue; // Already visited this edge

				if (!loop[edgeIndex])
					continue; // Edge isn't in our loop

				const MNEdge& newEdge = mesh.e[edgeIndex];
				if (edge.v1 != newEdge.v1 && edge.v1 != newEdge.v2 && edge.v2 != newEdge.v1 && edge.v2 != newEdge.v2)
					continue; // Edges aren't connected

				// Found a new edge we haven't visited that is in our loop, traverse it
				pathEdgeIndex = edgeIndex;
				break;
			}

			if (prevPathEdgeIndex == pathEdgeIndex)
			{
				// Our edge index didn't change, we couldn't find a new edge
				if (firstPass)
				{
					// We failed to find our end edge and ran out of loop edges, try the other way around
					edgesVisited.Clear(startEdge);
					pathEdgeIndex = startEdge;
					outPathEdges.ClearAll();
					pathLen = 0;
					firstPass = false;
				}
				else
				{
					// We failed to find our end edge on the loop from our start position, something went terribly wrong
					outPathEdges.ClearAll();
					pathLen = 0;
					break;
				}
			}
		}
	}

	if (firstPass)
	{
		// We found a valid path but might not be the shortest if we didn't explore the other way around, do so now

		// Reset our starting point so we can try again
		edgesVisited.Clear(startEdge);

		BitArray otherPathEdges;
		otherPathEdges.SetSize(mesh.nume);

		int otherPathLen = 0;

		int pathEdgeIndex = startEdge;
		while (!edgesVisited[pathEdgeIndex])
		{
			if (pathEdgeIndex == endEdge)
				break; // Found out what we were looking for

			edgesVisited.Set(pathEdgeIndex);
			otherPathEdges.Set(pathEdgeIndex);
			otherPathLen++;

			const int prevPathEdgeIndex = pathEdgeIndex;

			const MNEdge& edge = mesh.e[pathEdgeIndex];
			for (int edgeIndex = 0; edgeIndex < mesh.nume; ++edgeIndex)
			{
				if (edgesVisited[edgeIndex])
					continue; // Already visited this edge

				if (!loop[edgeIndex])
					continue; // Edge isn't in our loop

				const MNEdge& newEdge = mesh.e[edgeIndex];
				if (edge.v1 != newEdge.v1 && edge.v1 != newEdge.v2 && edge.v2 != newEdge.v1 && edge.v2 != newEdge.v2)
					continue; // Edges aren't connected

				// Found a new edge we haven't visited that is in our loop, traverse it
				pathEdgeIndex = edgeIndex;
				break;
			}

			if (prevPathEdgeIndex == pathEdgeIndex)
			{
				// Our vertex index didn't change, we couldn't find a new vertex
				// We failed to find our end vertex on the loop from our start position, use the first path we found
				otherPathLen = pathLen + 1; // Force to be longer so we don't take it
				break;
			}
		}

		if (otherPathLen < pathLen)
		{
			// Our new path is shorter, use it
			pathLen = otherPathLen;
			outPathEdges = otherPathEdges;
		}
	}

	return pathLen;
}


inline BitArray DoubleClickVertexSelection(MNMesh& mesh, bool add, bool sub, int prevSelectionHit, int currSelectionHit)
{
	// To compute our loop, we need a first vertex that is already selected, and a second vertex
	// to start the loop between. Because there might be multiple already selected vertices, we have
	// to choose a sensible one. We'll use the previously selected vertex and the current one.

	BitArray curSel;
	mesh.getVertexSel(curSel);

	BitArray newSel;

	// Make sure our last selection is valid and that it isn't our current hit vertex
	bool isLastSelectionValid = prevSelectionHit >= 0 && prevSelectionHit != currSelectionHit;
	if (isLastSelectionValid)
	{
		// Make sure it is part of the selection already if we add or not part of it if we remove
		if (add)
			isLastSelectionValid = curSel[prevSelectionHit];
		else if (sub)
			isLastSelectionValid = !curSel[prevSelectionHit];
	}

	const bool isAddingOrSubtracting = add || sub;
	if (isAddingOrSubtracting && isLastSelectionValid)
	{
		// Our loop target and output
		BitArray loop;
		loop.SetSize(mesh.numv);
		loop.Set(currSelectionHit);

		// The loop anchor is where the loop starts from our last hit
		BitArray loopAnchor;
		loopAnchor.SetSize(mesh.numv);
		loopAnchor.Set(prevSelectionHit);

		IMNMeshUtilities13* l_mesh13 = mesh.GetTypedInterface<IMNMeshUtilities13>();
		bool foundLoop = l_mesh13 != nullptr ? l_mesh13->FindLoopVertex(loop, loopAnchor) : false;
		if (foundLoop)
		{
			if (AreVerticesAdjacent(mesh, currSelectionHit, prevSelectionHit))
			{
				// Adjacent vertices add/remove the whole loop
				if (add)
					newSel = curSel | loop;
				else
					newSel = curSel & ~loop;
			}
			else
			{
				// Non-adjacent vertices add the partial loop between where we clicked and our current selection

				// Make sure our start/end points are part of the loop, they might not have been added
				loop.Set(prevSelectionHit);
				loop.Set(currSelectionHit);

				BitArray pathVertices;
				FindVertexLoopPath(mesh, loop, prevSelectionHit, currSelectionHit, pathVertices);

				if (add)
					newSel = curSel | pathVertices;
				else
					newSel = curSel & ~pathVertices;
			}
		}
	}
	else if (!isAddingOrSubtracting)
	{
		// We double clicked a vertex without adding/subtracting active, select vertices for the whole element
		newSel.SetSize(mesh.numv);
		newSel.Set(currSelectionHit);

		MNMeshSelectionConverter converter;

		// convert vertex hit to face
		BitArray hitFaces;
		converter.VertexToFace(mesh, newSel, hitFaces);

		int hitFace = -1;
		for (int faceIndex = 0; faceIndex < mesh.numf; ++faceIndex)
		{
			if (hitFaces[faceIndex])
			{
				hitFace = faceIndex;
				break;
			}
		}

		if (hitFace >= 0)
		{
			BitArray elementFaces;
			elementFaces.SetSize(mesh.numf);
			mesh.ElementFromFace(hitFace, elementFaces);

			for (int faceIndex = 0; faceIndex < mesh.numf; ++faceIndex)
			{
				if (!elementFaces[faceIndex])
					continue; // Face isn't part of our element

				// Add the face vertices to our selection set
				const MNFace& face = mesh.f[faceIndex];
				for (int faceVertexIndex = 0; faceVertexIndex < face.deg; ++faceVertexIndex)
					newSel.Set(face.vtx[faceVertexIndex]);
			}
		}
	}

	// If we haven't done anything with our new selection, just keep the current one
	if (newSel.GetSize() == 0)
		newSel = curSel;

	return newSel;
}

inline BitArray DoubleClickFaceSelection(MNMesh& mesh, bool add, bool sub, int prevSelectionHit, int currSelectionHit)
{
	// To compute our loop, we need a first face that is already selected, and a second face
	// to start the loop between. Because there might be multiple already selected faces, we have
	// to choose a sensible one. We'll use the previously selected face and the current one.

	BitArray curSel;
	mesh.getFaceSel(curSel);

	BitArray newSel;

	// Make sure our last selection is valid and that it isn't our current hit face
	bool isLastSelectionValid = prevSelectionHit >= 0 && prevSelectionHit != currSelectionHit;
	if (isLastSelectionValid)
	{
		// Make sure it is part of the selection already if we add or not part of it if we remove
		if (add)
			isLastSelectionValid = curSel[prevSelectionHit];
		else if (sub)
			isLastSelectionValid = !curSel[prevSelectionHit];
	}

	const bool isAddingOrSubtracting = add || sub;
	if (isAddingOrSubtracting && isLastSelectionValid)
	{
		// Our loop target and output
		BitArray loop;
		loop.SetSize(mesh.numf);
		loop.Set(currSelectionHit);

		// The loop anchor is where the loop starts from our last hit
		BitArray loopAnchor;
		loopAnchor.SetSize(mesh.numf);
		loopAnchor.Set(prevSelectionHit);

		IMNMeshUtilities13* l_mesh13 = mesh.GetTypedInterface<IMNMeshUtilities13>();
		bool foundLoop = l_mesh13 != nullptr ? l_mesh13->FindLoopFace(loop, loopAnchor) : false;
		if (foundLoop)
		{
			if (AreFacesAdjacent(mesh, currSelectionHit, prevSelectionHit))
			{
				// Adjacent faces add/remove the whole loop
				if (add)
					newSel = curSel | loop;
				else
					newSel = curSel & ~loop;
			}
			else
			{
				// Non-adjacent faces add the partial loop between where we clicked and our current selection

				// Make sure our start/end points are part of the loop, they might not have been added
				loop.Set(prevSelectionHit);
				loop.Set(currSelectionHit);

				BitArray pathFaces;
				FindFaceLoopPath(mesh, loop, prevSelectionHit, currSelectionHit, pathFaces);

				if (add)
					newSel = curSel | pathFaces;
				else
					newSel = curSel & ~pathFaces;
			}
		}
	}
	else if (!isAddingOrSubtracting)
	{
		// We double clicked a face without adding/subtracting active, select the whole element
		newSel.SetSize(mesh.numf);
		mesh.ElementFromFace(currSelectionHit, newSel);
	}

	// If we haven't done anything with our new selection, just keep the current one
	if (newSel.GetSize() == 0)
		newSel = curSel;

	return newSel;
}

inline BitArray FindFaceLoop(MNMesh& mesh, int startFace, int endFace)
{
	BitArray loop;
	loop.SetSize(mesh.numf);

	if (startFace < 0 || endFace < 0)
		return loop;

	loop.Set(endFace);

	// The loop anchor is where the loop starts
	BitArray loopAnchor;
	loopAnchor.SetSize(mesh.numf);
	loopAnchor.Set(startFace);

	IMNMeshUtilities13* l_mesh13 = mesh.GetTypedInterface<IMNMeshUtilities13>();
	bool foundLoop = l_mesh13 != nullptr ? l_mesh13->FindLoopFace(loop, loopAnchor) : false;
	if (!foundLoop)
		loop.ClearAll();

	return loop;
}

inline BitArray FindEdgesFromFaces(MNMesh& mesh, const BitArray& faces)
{
	BitArray edges;
	edges.SetSize(mesh.nume);
	if (faces.NumberSet() == 0)
		return edges;

	for (int faceIndex = 0; faceIndex < mesh.numf; ++faceIndex)
	{
		if (!faces[faceIndex])
			continue; // Not selected

		const MNFace& face = mesh.f[faceIndex];
		for (int faceEdgeIndex = 0; faceEdgeIndex < face.deg; ++faceEdgeIndex)
			edges.Set(face.edg[faceEdgeIndex]);
	}

	return edges;
}

inline BitArray DoubleClickEdgeSelection(MNMesh& mesh, bool add, bool sub, int prevSelectionHit, int currSelectionHit, bool useLoopSelection, bool& outFoundPartialPath)
{
	// To compute our loop, we need a first edge that is already selected, and a second edge
	// to start the loop between. Because there might be multiple already selected edges, we have
	// to choose a sensible one. We'll use the previously selected edge and the current one.

	BitArray curSel;
	mesh.getEdgeSel(curSel);

	BitArray newSel;

	// Make sure our last selection is valid and that it isn't our current hit edge
	bool isLastSelectionValid = prevSelectionHit >= 0 && prevSelectionHit != currSelectionHit;
	if (isLastSelectionValid)
	{
		// Make sure it is part of the selection already if we add or not part of it if we remove
		if (add)
			isLastSelectionValid = curSel[prevSelectionHit];
		else if (sub)
			isLastSelectionValid = !curSel[prevSelectionHit];
	}

	const bool isAddingOrSubtracting = add || sub;

	// Look for a partial path first if we aren't exclusively using loop selection
	bool foundPartialPath = false;
	if (isAddingOrSubtracting && isLastSelectionValid && !useLoopSelection)
	{
		// Our loop target and output
		BitArray loop;
		loop.SetSize(mesh.nume);
		loop.Set(currSelectionHit);

		// The loop anchor is where the loop starts from our last hit
		BitArray loopAnchor;
		loopAnchor.SetSize(mesh.nume);
		loopAnchor.Set(prevSelectionHit);

		IMNMeshUtilities13* l_mesh13 = mesh.GetTypedInterface<IMNMeshUtilities13>();
		const bool foundLoop = l_mesh13->FindLoopOrRingEdge(loop, loopAnchor);

		if (foundLoop)
		{
			bool isRing = false;
			if (AreEdgesAdjacent(mesh, currSelectionHit, prevSelectionHit, isRing))
			{
				// Adjacent edges add/remove the whole loop
				if (add)
					newSel = curSel | loop;
				else
					newSel = curSel & ~loop;

				foundPartialPath = true;
			}
			else
			{
				// Non-adjacent edges add the partial loop between where we clicked and our current selection

				// Make sure our start/end points are part of the loop, they might not have been added
				loop.Set(prevSelectionHit);
				loop.Set(currSelectionHit);

				// Edge loops are connected through vertices
				BitArray pathLoopEdges;
				int numLoopEdges = FindEdgeLoopPathThroughVertices(mesh, loop, prevSelectionHit, currSelectionHit, pathLoopEdges);

				// Edge rings are connected through faces
				BitArray pathRingEdges;
				int numRingEdges = FindEdgeLoopPathThroughFaces(mesh, loop, prevSelectionHit, currSelectionHit, pathRingEdges);

				// Find our shortest path, if any
				BitArray shortestLoopPath;
				if (numLoopEdges > 0)
				{
					if (numRingEdges > 0)
						shortestLoopPath = numLoopEdges < numRingEdges ? pathLoopEdges : pathRingEdges;	// Both the loop and ring paths were valid, pick the shortest
					else
						shortestLoopPath = pathLoopEdges;	// Only the loop path is valid, use it
				}
				else if (numRingEdges > 0)
					shortestLoopPath = pathRingEdges;		// Only the ring path is valid, use it
				else
					shortestLoopPath.SetSize(mesh.nume);	// Neither path was valid, leave the current selection unchanged

				if (add)
					newSel = curSel | shortestLoopPath;
				else
					newSel = curSel & ~shortestLoopPath;

				foundPartialPath = true;
			}
		}
	}

	outFoundPartialPath = foundPartialPath;

	if (isAddingOrSubtracting && !foundPartialPath)
	{
		// We double clicked without any single clicks in between, use loop selection
		BitArray edgeLoop;
		edgeLoop.SetSize(mesh.nume);
		edgeLoop.Set(currSelectionHit);

		TemporaryNoBadVerts bv(mesh);
		mesh.SelectEdgeLoop(edgeLoop);

		// Found our edge loop, combine it with our current selection
		if (add)
			newSel = curSel | edgeLoop;
		else
			newSel = curSel & ~edgeLoop;
	}

	if (!isAddingOrSubtracting)
	{
		// We double clicked an edge without adding/subtracting active, select its edge loop
		newSel.SetSize(mesh.nume);
		newSel.Set(currSelectionHit);

		// If there are "bad verts", such as verts welded on open edges, which we allow temporarily, the SelectEdgeLoop
		// call can alter topology underneath us. We use the TemporaryNoBadVerts helper class to temporarily allow
		// bad verts so that can't happen.
		TemporaryNoBadVerts bv(mesh);
		mesh.SelectEdgeLoop(newSel);
	}

	// If we haven't done anything with our new selection, just keep the current one
	if (newSel.GetSize() == 0)
		newSel = curSel;

	return newSel;
}

class DoubleClickSelector
{
public:
	DoubleClickSelector()
	{
		Reset();
	}

	void OnSingleClickVertex(int vertexIndex)
	{
		if (vertexIndex == mLastSubObjectHits[0])
			return;	// Already recorded

		mLastSubObjectHits[1] = mLastSubObjectHits[0];
		mLastSubObjectHits[0] = vertexIndex;
	}

	void OnSingleClickEdge(int edgeIndex)
	{
		if (edgeIndex != mLastSubObjectHits[0])
		{
			mLastSubObjectHits[1] = mLastSubObjectHits[0];
			mLastSubObjectHits[0] = edgeIndex;
		}

		// Even if the current click is on the same edge as the last click, we still
		// want to update our state. We might double click an edge to add its loop/ring.
		// Then while holding ALT, single click it, and double click another edge on the loop/ring
		// to remove between the two points.

		switch (mEdgeState)
		{
		case EdgeSelectionState::Root:
			// Received our first single click, not sure yet if it'll be a double click
			// If it's a double click, we'll do loop selection otherwise the next single click will
			// bring us to point to point selection
			mEdgeState = EdgeSelectionState::MaybeLoopSelection;
			break;
		case EdgeSelectionState::MaybeLoopSelection:
			// Already received one single click and now got a second one, we'll be in point to point selection
			mEdgeState = EdgeSelectionState::PointToPoint;
			break;
		case EdgeSelectionState::PointToPoint:
			// Once in point to point, we never leave unless we clear the selection or we fail to find a
			// valid path between our two points.
			break;
		}
	}

	void OnSingleClickFace(int faceIndex)
	{
		if (faceIndex == mLastSubObjectHits[0])
			return; // Already recorded

		mLastSubObjectHits[1] = mLastSubObjectHits[0];
		mLastSubObjectHits[0] = faceIndex;
	}

	BitArray OnDoubleClickVertex(MNMesh& mesh, bool add, bool sub, int vertexIndex)
	{
		const int prevSelectionHit = mLastSubObjectHits[1]; // Second to last hit since we double clicked
		const int currSelectionHit = vertexIndex;

		return DoubleClickVertexSelection(mesh, add, sub, prevSelectionHit, currSelectionHit);
	}

	BitArray OnDoubleClickEdge(MNMesh& mesh, bool add, bool sub, int edgeIndex)
	{
		bool useLoopSelection = true;

		switch (mEdgeState)
		{
		case EdgeSelectionState::MaybeLoopSelection:
			// We received a double click, use loop selection
			useLoopSelection = true;
			mEdgeState = EdgeSelectionState::Root;
			break;
		case EdgeSelectionState::PointToPoint:
			// We use point to point selection
			useLoopSelection = false;
			break;
		}

		const int prevSelectionHit = mLastSubObjectHits[1]; // Second to last hit since we double clicked
		const int currSelectionHit = edgeIndex;

		bool foundPartialPath = false;
		BitArray newSel = DoubleClickEdgeSelection(mesh, add, sub, prevSelectionHit, currSelectionHit, useLoopSelection, foundPartialPath);
		
		if (mEdgeState == EdgeSelectionState::PointToPoint && !foundPartialPath)
		{
			// We were looking for a point to point path but failed, bail out of partial loop selection
			// since we performed a loop selection instead
			mEdgeState = EdgeSelectionState::Root;
		}

		return newSel;
	}

	BitArray OnDoubleClickFace(MNMesh& mesh, bool add, bool sub, int faceIndex)
	{
		const int prevSelectionHit = mLastSubObjectHits[1]; // Second to last hit since we double clicked
		const int currSelectionHit = faceIndex;

		return DoubleClickFaceSelection(mesh, add, sub, prevSelectionHit, currSelectionHit);
	}

	void Reset()
	{
		mLastSubObjectHits[0] = mLastSubObjectHits[1] = -1;
		mEdgeState = EdgeSelectionState::Root;
	}

private:
	enum class EdgeSelectionState
	{
		Root,					// No click information yet
		MaybeLoopSelection,		// Received one single click, if it's a double click we'll do loop selection
		PointToPoint,			// Received a second single click, we'll do point to point selection now
	};

	// Last two vert/edge/face hits, index 0 is most recent
	int mLastSubObjectHits[2];

	// Edge selection requires a state machine
	EdgeSelectionState mEdgeState;
};
