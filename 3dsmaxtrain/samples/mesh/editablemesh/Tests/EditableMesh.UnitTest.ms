--------------------------------------------------------------------------------------
-- Tests for mesh manipulation through the Edit Mesh object.
--
-- Author: Lee Betchen
-- Created: October 2021
--------------------------------------------------------------------------------------

Struct EditableMeshUnitTests
( 
	Function setup = 
	( 
	),

	Function teardown = 
	( 
	),

	Function EditableMeshNormalsTests = 
	(
		local nDim = 3
		local nBoxesAttachTest = 2

		-- Test preservation and propagation of specified or explicit normal vectors through Editable Mesh operations
		-- ==========================================================================================================
		format "Editable Mesh -- Normals Tests\n"

		-- Test an attach operation between objects with applied transformations, to ensure that normals are transformed
		-- properly; create two boxes, and generate explicit normals on the second with the Weighted Normals modifier
		resetMaxFile #noprompt
		boxes = #()
		boxes.count = nBoxesAttachTest
		for b = 1 to nBoxesAttachTest do
		(
			xPosition = 100.0 * (b - 1)
			boxes[b] = Box lengthsegs:1 widthsegs:1 heightsegs:1 pos:[xPosition,0.0,0.0] isSelected:on length:50.0 width:50.0 height:50.0 mapCoords:true
			convertTo $ TriMeshGeometry
		)

		select boxes[2]
		max modify mode
		addModifier $ (WeightedNormalsMod())
		convertTo $ TriMeshGeometry

		-- Apply a rotation to the first box, and a non-uniform scale to the second
		select boxes[1]
		for i = 1 to nDim do
		(
			axisVec = [0.0,0.0,0.0]
			axisVec[i] = 1.0
			rotate $ (angleaxis -30.0 axisVec)
		)

		select boxes[2]
		scale $ [1.0,5.0,1.0]
		
		-- Add attach the second boxes to the first
		select boxes[1]
		meshop.attach $.baseObject boxes[2] targetNode:$

		-- Check vertex normals over a selection of faces on the second box
		addModifier $ (Edit_Normals())
		faceIndices = #(15,16,17,18,23,24)
		normalVecs = #(#([0.815,0.239,0.529],[-0.236,-0.67,0.704],[-0.114,-0.845,0.522]),#([-0.114,-0.845,0.522],[0.936,0.064,0.347],[0.815,0.239,0.529]),#([0.936,0.064,0.347],[-0.114,-0.845,0.522],[-0.815,-0.239,-0.529]),#([-0.815,-0.239,-0.529],[0.236,0.67,-0.704],[0.936,0.064,0.347]), #([-0.114,-0.845,0.522],[-0.236,-0.67,0.704],[-0.936,-0.064,-0.347]),#([-0.936,-0.064,-0.347],[-0.815,-0.239,-0.529],[-0.114,-0.845,0.522]))
		nFacesCompared = faceIndices.count
		for f = 1 to nFacesCompared do
		(
			nVerticesFace = ($.modifiers[1]).GetFaceDegree faceIndices[f]
			nVerticesFaceExpected = (normalVecs[f]).count
			assert_equal nVerticesFaceExpected nVerticesFace message: ("Error Test 1 (face degree, face " + (faceIndices[f] as string) + ")")
			for v = 1 to nVerticesFace do
			(
				indexNormal = ($.modifiers[1]).GetNormalID faceIndices[f] v
				assert_point3_equal normalVecs[f][v] (($.modifiers[1]).GetNormal indexNormal) tolerance:0.001 message: ("Error Test 1 (normal vector, face " + (faceIndices[f] as string) + ", vertex " + (v as string) + ")")
			)
		)
	),

	Tests = #(EditableMeshNormalsTests)
)

if (run_test_fixture == undefined) then 
( 
	format "-- No fixture!\n" 
	testList = EditableMeshUnitTests() 
	for test in testList.Tests do
	(
		testList.setup()
		test()
		testList.teardown()
	)
) 
else 
( 
	format "-- Fixture found!\n" 
	run_test_fixture EditableMeshUnitTests script:(getThisScriptFilename())
) 
