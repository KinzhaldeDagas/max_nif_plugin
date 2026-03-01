fileIn ((getFileNamePath(getSourceFileName())) + "/CompareWithBenchmark.mxs")

--------------------------------------------------------------------------------------
-- Tests for manipulation of vertex normals via the Edit Normals modifier.
--
-- Author: Lee Betchen
-- Created: May 2021
--------------------------------------------------------------------------------------

Struct EditNormalsUnitTests 
( 
	Function setup = 
	( 
	),

	Function teardown = 
	( 
	),

	Function GetListFromBitArray &selectedIndices &selectionFlags =
	(
		-- Return selectedIndices as an ordered list of all indices i for which selectionFlags[i] is true
		selectedIndices.count = 0
		nFlags = selectionFlags.count
		for i = 1 to nFlags do
		(
			if (selectionFlags[i]) do
			(
				append selectedIndices i
			)
		)
	),

	Function EditNormalsSelectionTests = 
	(
		local nCases = 2
		local nBoxes = 2
		local selectedFaces = #(23,24)
		local expectedSelectionNormals = #(3,13,14)
		local expectedSelectionVertices = #(#(16),#(17,45,53),#(16),#(17,45,53))
		local expectedSelectionEdges = #(#(10,13,16),#(10,13,17,52,53,54),#(5,6,8,9),#(3,5,8,9))
		local expectedSelectionFaces = #(#(10,13,16,19,25,26),#(48,49,51,52,53,54),#(11,12,20,21),#(21,24,25,27))
		local boxPositions = #([0.0,0.0,0.0],[100.0,0.0,0.0])
		local isPolyCase = #(true,false)
		local isInitiallySpecified = #(true,false)
		local isInitiallyExplicit = #(true,false)

		-- Test selection options for normal vectors
		-- =========================================
		format "Edit Normals -- Selection Tests\n"

		-- Generate a model consisting of a simple box, and a box with explicit normals defined
		resetMaxFile #noprompt
		boxes = #()
		boxes.count = nBoxes
		for b = 1 to nBoxes do
		(
			boxes[b] = Box lengthsegs:2 widthsegs:2 heightsegs:2 pos:boxPositions[b] isSelected:on length:50.0 width:50.0 height:50.0 mapCoords:true
		)
		
		select boxes[1]
		max modify mode
		addModifier $ (WeightedNormalsMod())

		-- For each box, test selection options in two cases: after conversion to an Editable Poly, and to an Editable Mesh
		nFacesSelected = selectedFaces.count
		nNormalsSelected = expectedSelectionNormals.count
		for c = 1 to nCases do
		(
			for b = 1 to nBoxes do
			(
				indexTest = (c - 1) * nBoxes + b
				select boxes[b]
				
				-- Convert box b to Editable Poly or Editable Mesh, as appropriate for case c
				if (isPolyCase[c]) then
				(
					convertTo $ PolyMeshObject
				)
				else
				(
					convertTo $ TriMeshGeometry
				)

				-- Add an Edit Normals modifier
				addModifier $ (EditNormals())

				-- Select a group of normals one-by-one, using the normal index directly
				normalSelectionFlags = #{}
				($.modifiers[1]).SetSelection &normalSelectionFlags
				for n = 1 to nNormalsSelected do
				(
					normalSelectionFlags = #{expectedSelectionNormals[n]}
					($.modifiers[1]).Select &normalSelectionFlags
				)

				-- Fetch the current selection as a BitArray, convert to a list of indices of selected normals, and compare
				-- with our benchmark
				selectionCurr = #()
				normalSelectionFlags = ($.modifiers[1]).GetSelection()
				GetListFromBitArray &selectionCurr &normalSelectionFlags
				Assert_array_equal expectedSelectionNormals selectionCurr message: ("Error Selection Test " + (indexTest as string) + " (selection by normal)")

				-- Select all normals at a particular vertex, and compare the selected normal indices with our benchmark
				normalSelectionFlags = #{}
				vertexSelectionFlags = #{16}
				($.modifiers[1]).ConvertVertexSelection &vertexSelectionFlags &normalSelectionFlags
				($.modifiers[1]).SetSelection &normalSelectionFlags
				normalSelectionFlags = ($.modifiers[1]).GetSelection()
				GetListFromBitArray &selectionCurr &normalSelectionFlags
				Assert_array_equal expectedSelectionVertices[indexTest] selectionCurr message: ("Error Selection Test " + (indexTest as string) + " (selection by vertex)")

				-- Select all normals along a particular pair of edges, and compare the selected normal indices with our
				-- benchmark
				normalSelectionFlags = #{}
				edgeSelectionFlags = #{16,22}
				($.modifiers[1]).ConvertEdgeSelection &edgeSelectionFlags &normalSelectionFlags
				($.modifiers[1]).SetSelection &normalSelectionFlags
				normalSelectionFlags = ($.modifiers[1]).GetSelection()
				GetListFromBitArray &selectionCurr &normalSelectionFlags
				Assert_array_equal expectedSelectionEdges[indexTest] selectionCurr message: ("Error Selection Test " + (indexTest as string) + " (selection by edge)")

				-- Select all normals on a particular pair of faces, and compare the selected normal indices with our benchmark
				normalSelectionFlags = #{}
				faceSelectionFlags = #{}
				nFaces = ($.modifiers[1]).GetNumFaces()
				faceSelectionFlags.count = nFaces
				for f = 1 to nFaces do
				(
					faceSelectionFlags[f] = false
				)

				for f = 1 to nFacesSelected do
				(
					faceSelectionFlags[selectedFaces[f]] = true
				)

				($.modifiers[1]).ConvertFaceSelection &faceSelectionFlags &normalSelectionFlags
				($.modifiers[1]).SetSelection &normalSelectionFlags
				normalSelectionFlags = ($.modifiers[1]).GetSelection()
				GetListFromBitArray &selectionCurr &normalSelectionFlags
				Assert_array_equal expectedSelectionFaces[indexTest] selectionCurr message: ("Error Selection Test " + (indexTest as string) + " (selection by face)")

				-- Check to ensure that the normal used at each vertex of each face of the selected pair above is in the
				-- expected initial state
				for f = 1 to nFacesSelected do
				(
					nVerticesFace = ($.modifiers[1]).GetFaceDegree selectedFaces[f]
					for v = 1 to nVerticesFace do
					(
						isSpecified = ($.modifiers[1]).GetFaceNormalSpecified selectedFaces[f] v
						indexNormal = ($.modifiers[1]).GetNormalID selectedFaces[f] v
						isExplicit = ($.modifiers[1]).GetNormalExplicit indexNormal
						Assert_equal isSpecified isInitiallySpecified[b] message: ("Error Selection Test " + (indexTest as string) + " (initial face vertex normal specified flag, face " + (selectedFaces[f] as string) + ", vertex " + (v as string) + ")")
						Assert_equal isExplicit isInitiallyExplicit[b] message: ("Error Selection Test " + (indexTest as string) + " (initial face vertex normal explicit flag, face " + (selectedFaces[f] as string) + ", vertex " + (v as string) + ")")
					)
				)

				-- Set all selected normals to specified, and check each corner of each face in our selected pair
				($.modifiers[1]).Specify()
				for f = 1 to nFacesSelected do
				(
					nVerticesFace = ($.modifiers[1]).GetFaceDegree selectedFaces[f]
					for v = 1 to nVerticesFace do
					(
						isSpecified = ($.modifiers[1]).GetFaceNormalSpecified selectedFaces[f] v
						Assert_true isSpecified message: ("Error Selection Test " + (indexTest as string) + " (face vertex normal specified flag, face " + (selectedFaces[f] as string) + ", vertex " + (v as string) + ")")
					)
				)

				-- Set all selected normals to explicit, and check each corner of each face in our selected pair
				($.modifiers[1]).MakeExplicit()
				for f = 1 to nFacesSelected do
				(
					nVerticesFace = ($.modifiers[1]).GetFaceDegree selectedFaces[f]
					for v = 1 to nVerticesFace do
					(
						indexNormal = ($.modifiers[1]).GetNormalID selectedFaces[f] v
						isExplicit = ($.modifiers[1]).GetNormalExplicit indexNormal
						Assert_true isExplicit message: ("Error Selection Test " + (indexTest as string) + " (face vertex normal explicit flag, face " + (selectedFaces[f] as string) + ", vertex " + (v as string) + ")")
					)
				)

				-- Reset all selected normals to default treatment, and check each corner of each face in our selected pair
				($.modifiers[1]).Reset()
				for f = 1 to nFacesSelected do
				(
					nVerticesFace = ($.modifiers[1]).GetFaceDegree selectedFaces[f]
					for v = 1 to nVerticesFace do
					(
						isSpecified = ($.modifiers[1]).GetFaceNormalSpecified selectedFaces[f] v
						indexNormal = ($.modifiers[1]).GetNormalID selectedFaces[f] v
						isExplicit = ($.modifiers[1]).GetNormalExplicit indexNormal
						Assert_false isSpecified message: ("Error Selection Test " + (indexTest as string) + " (reset face vertex normal specified flag, face " + (selectedFaces[f] as string) + ", vertex " + (v as string) + ")")
						Assert_false isExplicit message: ("Error Selection Test " + (indexTest as string) + " (reset face vertex normal explicit flag, face " + (selectedFaces[f] as string) + ", vertex " + (v as string) + ")")
					)
				)

				-- Remove the Edit Normals modifier
				deleteModifier $ 1
			)
		)
	),

	Function EditNormalsUnifyBreakTests =
	(
		local nCases = 2
		local doAddWeightedNormals = #(false,true)
		
		-- Test Unify/Break functionality for normal vectors at a vertex
		-- =============================================================
		format "Edit Normals -- Unify/Break Tests\n"

		-- Test all subcase both without and with explicit normals defined on the input mesh
		for c = 1 to nCases do
		(
			-- Generate a simple one-segment box, which by default will have three distinct normals at each vertex, and apply an
			-- Edit Normals modifier
			resetMaxFile #noprompt
			boxCurr = Box lengthsegs:1 widthsegs:1 heightsegs:1 pos:[0.0,0.0,0.0] isSelected:on length:50.0 width:50.0 height:50.0 mapCoords:true
			select boxCurr
			convertTo $ PolyMeshObject
			max modify mode
			if (doAddWeightedNormals[c]) do
			(
				addModifier $ (WeightedNormalsMod useSmoothingGroups:true)
			)

			addModifier $ (EditNormals())

			-- Unify all threee normals at one vertex, and two of three at another, without averaging; this will generate
			-- specified normals
			normalSelectionFlags = #{5,8,12,23,24}
			($.modifiers[1]).SetSelection &normalSelectionFlags
			($.modifiers[1]).Unify toAverage:false

			-- Compare with benchemark results
			vertexList = #()
			faceList = #()
			nFacesBox = ($.modifiers[1]).GetNumFaces()
			faceList.count = nFacesBox
			for f = 1 to nFacesBox do
			(
				faceList[f] = f
			)

			caseName = (c as string) + "a"
			CompareWithBenchmark ("EditNormalsUnifyBreakTest" + caseName + ".max") caseName 0.0 boxCurr.name false &vertexList &faceList 1 false true

			-- Rebuild the model above, unify the same normals but this time with averaging, resulting in explicit normals, and
			-- compare with benchmark results
			resetMaxFile #noprompt
			boxCurr = Box lengthsegs:1 widthsegs:1 heightsegs:1 pos:[0.0,0.0,0.0] isSelected:on length:50.0 width:50.0 height:50.0 mapCoords:true
			select boxCurr
			convertTo $ PolyMeshObject
			max modify mode
			if (doAddWeightedNormals[c]) do
			(
				addModifier $ (WeightedNormalsMod useSmoothingGroups:true)
			)

			addModifier $ (EditNormals())
			($.modifiers[1]).SetSelection &normalSelectionFlags
			($.modifiers[1]).Unify toAverage:true
			caseName = (c as string) + "b"
			CompareWithBenchmark ("EditNormalsUnifyBreakTest" + caseName + ".max") caseName 0.0 boxCurr.name false &vertexList &faceList 1 false true

			-- Generate a sphere, which by default will have a single shared normal at each vertex, and apply an Edit Normals
			-- modifier
			resetMaxFile #noprompt
			sphereCurr = Sphere segs:32 pos:[0.0,0.0,0.0] isSelected:on radius:25.0 mapCoords:true
			select sphereCurr
			convertTo $ PolyMeshObject
			max modify mode
			if (doAddWeightedNormals[c]) do
			(
				addModifier $ (WeightedNormalsMod useSmoothingGroups:true)
			)

			addModifier $ (EditNormals())

			-- Break all normals on the "top" half of the sphere without averaging, generating a distinct specified normal at
			-- each face corner
			normalSelectionFlags = #{}
			nNormalsInit = ($.modifiers[1]).GetNumNormals()
			normalSelectionFlags.count = nNormalsInit
			nNormalsToBreak = nNormalsInit / 2
			for n = 1 to nNormalsToBreak do
			(
				normalSelectionFlags[n] = true
			)
			
			for n = (nNormalsToBreak + 1) to nNormalsInit do
			(
				normalSelectionFlags[n] = false
			)
			
			($.modifiers[1]).SetSelection &normalSelectionFlags
			($.modifiers[1]).Break toAverage:false

			-- Compare with benchmark results
			nFacesSphere = ($.modifiers[1]).GetNumFaces()
			faceList.count = nFacesSphere
			for f = 1 to nFacesSphere do
			(
				faceList[f] = f
			)

			caseName = (c as string) + "c"
			CompareWithBenchmark ("EditNormalsUnifyBreakTest" + caseName + ".max") caseName 0.0 sphereCurr.name false &vertexList &faceList 1 false true

			-- Rebuild the model above, breaking the same normals but this time with averaging, resulting in equivalent
			-- explicit normals at each face corner about a vertex, and compare with benchmark results
			resetMaxFile #noprompt
			sphereCurr = Sphere segs:32 pos:[0.0,0.0,0.0] isSelected:on radius:25.0 mapCoords:true
			select sphereCurr
			convertTo $ PolyMeshObject
			max modify mode
			if (doAddWeightedNormals[c]) do
			(
				addModifier $ (WeightedNormalsMod useSmoothingGroups:true)
			)

			addModifier $ (EditNormals())
			($.modifiers[1]).SetSelection &normalSelectionFlags
			($.modifiers[1]).Break toAverage:true
			caseName = (c as string) + "d"
			CompareWithBenchmark ("EditNormalsUnifyBreakTest" + caseName + ".max") caseName 0.0 sphereCurr.name false &vertexList &faceList 1 false true
		)
	),

	Function EditNormalsAveragingTests =
	(
		local nBoxes = 2
		local nRowsSphere = 16
		local nFacesSphereRow = 32
		local boxPositions = #([0.0,0.0,0.0],[100.0,0.0,0.0])
		
		-- Test averaging functionality for vertex normals
		-- ===============================================
		format "Edit Normals -- Average Tests\n"

		-- First test; generate a simple model consisting of a sphere, and apply an Edit Normals modifier
		resetMaxFile #noprompt
		sphereCurr = Sphere segs:nFacesSphereRow pos:[0.0,0.0,0.0] isSelected:on radius:25.0 mapCoords:true
		select sphereCurr
		convertTo $ PolyMeshObject
		max modify mode
		addModifier $ (EditNormals())

		-- Average all normals on the "top" hemisphere of our sphere without a threshold, resulting in all vertices using an
		-- identical explicit normal in the positive z-direction
		normalSelectionFlags = #{}
		nNormalsInit = ($.modifiers[1]).GetNumNormals()
		normalSelectionFlags.count = nNormalsInit
		nNormalsHemisphere = nNormalsInit / 2
		for n = 1 to nNormalsHemisphere do
		(
			normalSelectionFlags[n] = true
		)

		for n = (nNormalsHemisphere + 1) to nNormalsInit do
		(
			normalSelectionFlags[n] = false
		)

		($.modifiers[1]).SetSelection &normalSelectionFlags
		($.modifiers[1]).Average useThresh:false

		-- Compare with benchmark results
		vertexList = #()
		faceList = #()
		faceList.count = nRowsSphere
		for r = 1 to nRowsSphere do
		(
			faceList[r] = 1 + (r - 1) * nFacesSphereRow
		)

		CompareWithBenchmark "EditNormalsAveragingTest1.max" "1" 0.0 sphereCurr.name false &vertexList &faceList 1 false true

		-- Second test; average all normals on the "top" hemisphere of our sphere with a threshold, resulting in all vertices
		-- using explicit normals equivalent to their original default normals, and compare with benchmark results
		resetMaxFile #noprompt
		sphereCurr = Sphere segs:nFacesSphereRow pos:[0.0,0.0,0.0] isSelected:on radius:25.0 mapCoords:true
		select sphereCurr
		convertTo $ PolyMeshObject
		max modify mode
		addModifier $ (EditNormals())
		($.modifiers[1]).SetSelection &normalSelectionFlags
		($.modifiers[1]).Average useThresh:true threshold:0.1
		CompareWithBenchmark "EditNormalsAveragingTest2.max" "2" 0.0 sphereCurr.name false &vertexList &faceList 1 false true

		-- Third test; create a model with two simple one-segment boxes, add a Weighted Normals modifier to one box, and add an
		-- Edit Normals modifier to each
		resetMaxFile #noprompt
		boxes = #()
		boxes.count = nBoxes
		for b = 1 to nBoxes do
		(
			boxes[b] = Box lengthsegs:1 widthsegs:1 heightsegs:1 pos:boxPositions[b] isSelected:on length:50.0 width:50.0 height:50.0 mapCoords:true
		)

		select boxes[1]
		max modify mode
		addModifier $ (WeightedNormalsMod())
		select #(boxes[1],boxes[2])
		modPanel.addModToSelection (EditNormals()) ui:on

		-- Average an explicit normal from the first box with a default normal from the second, resulting in an identical
		-- explicit normal on each box
		((boxes[1]).modifiers[1]).AverageTwo boxes[1] 5 boxes[2] 23

		-- Compare with benchmark results
		nFacesBox = ((boxes[1]).modifiers[1]).GetNumFaces node:boxes[1]
		faceList.count = nFacesBox
		for f = 1 to nFacesBox do
		(
			faceList[f] = f
		)

		CompareWithBenchmark "EditNormalsAveragingTest3.max" "3a" 0.0 (boxes[1]).name true &vertexList &faceList 1 false true
		CompareWithBenchmark "EditNormalsAveragingTest3.max" "3b" 0.0 (boxes[2]).name false &vertexList &faceList 1 false true
	),

	Function EditNormalsSetSaveLoadTests =
	(
		nCases = 2
		local boxObjectFacesAltered = #(552,553)
		local normalVec = [0.0,-1.0,0.0]
		local useSingleModifier = #(false,true)

		-- Test face vertex normal setting through Edit Normals, and save/load functionality
		-- =================================================================================
		format "Edit Normals -- Set Normal and Save/Load Tests\n"

		-- Test direct setting of normals on multiple, more geometrically complex objects, as well as save/load operations,
		-- box with separate Edit Normals modifiers on each object, as well as a single shared modifier
		for c = 1 to nCases do
		(
			-- Load a model with two objects having more complex geometry
			resetMaxFile #noprompt
			local filePath = getFileNamePath(getSourceFileName())
			local fileName = filePath + "/TestData/EditNormalsTestModel.max"
			local isLoaded = loadMaxFile fileName quiet:true
			Assert_true isLoaded message: ("Error Set Normal and Save/Load Test " + (c as string) + " (load)")
			if (isLoaded) do
			(
				-- Apply a Weighted Normals modifier to the perforated cylinder, with smoothing
				select $PerforatedCylinder
				max modify mode
				addModifier $ (WeightedNormalsMod useHardEdgeAngle:true smoothingCoeff:1.0 boundaryCoeff:1.0)

				-- Apply either a single modifier to both objects, or one modifier to each, as appropriate for case c
				if (useSingleModifier[c]) then
				(
					select #($BoxObject,$PerforatedCylinder)
					modPanel.addModToSelection (EditNormals()) ui:on
				)
				else
				(
					addModifier $ (EditNormals())
					select $BoxObject
					addModifier $ (EditNormals())
				)

				-- Set all normals used by the flat part of the "front" of object BoxObject to be precisely normal to that
				-- surface, eliminating the effects of the spherical cutouts, and make these normals explicit
				select $BoxObject
				nBoxObjectFacesAltered = boxObjectFacesAltered.count
				for f = 1 to nBoxObjectFacesAltered do
				(
					nVerticesFace = ($.modifiers[1]).GetFaceDegree boxObjectFacesAltered[f] node:$BoxObject
					for v = 1 to nVerticesFace do
					(
						indexNormal = ($.modifiers[1]).GetNormalID boxObjectFacesAltered[f] v node:$BoxObject
						($.modifiers[1]).SetNormalExplicit indexNormal explicit:true node:$BoxObject
						($.modifiers[1]).SetNormal indexNormal &normalVec node:$BoxObject
					)
				)

				-- Adjust one of the explicit normals on the outer surface of the PerforatedCylinder object
				select $PerforatedCylinder
				($.modifiers[1]).SetNormal 2269 &normalVec node:$PerforatedCylinder

				-- Compare object BoxObject with the benchmark results
				vertexList = #()
				faceListBoxObject = #()
				select $BoxObject
				nFacesBoxObject = ($.modifiers[1]).GetNumFaces node:$BoxObject
				faceListBoxObject.count = nFacesBoxObject
				for f = 1 to nFacesBoxObject do
				(
					faceListBoxObject[f] = f
				)

				CompareWithBenchmark "EditNormalsSetSaveLoadTestBenchmark.max" ((c as string) + "a") 0.0 ($BoxObject).name true &vertexList &faceListBoxObject 1 false true

				-- Compare object PerforatedCylinder with the benchmark results
				faceListPerforatedCylinder = #()
				select $PerforatedCylinder
				nFacesPerforatedCylinder = ($.modifiers[1]).GetNumFaces node:$PerforatedCylinder
				faceListPerforatedCylinder.count = nFacesPerforatedCylinder
				for f = 1 to nFacesPerforatedCylinder do
				(
					faceListPerforatedCylinder[f] = f
				)
				
				CompareWithBenchmark "EditNormalsSetSaveLoadTestBenchmark.max" ((c as string) + "b") 0.0 ($PerforatedCylinder).name true &vertexList &faceListPerforatedCylinder 1 false true

				-- Test save and load of Edit Normals modifier; first, save a temporary file
				isSaved = false
				filePath = GetDir #temp
				fileName = filePath + "/TempFile_EditNormalsSetSaveLoadTest.max"
				isNameUnavailable = isMaxFile fileName
				if (not isNameUnavailable) do
				(
					isSaved = saveMaxFile fileName quiet:true
				)

				Assert_true isSaved message: ("Error Set Normal and Save/Load Test " + (c as string) + " (save)")
				if (isSaved) do
				(
					-- Next, load the file
					resetmaxFile #noPrompt
					isLoaded = loadMaxFile fileName quite:true
					SceneConverter.visible = false
					Assert_true isLoaded message: ("Error Set Normal and Save/Load Test " + (c as string) + " (load from saved)")
					if (isLoaded) do
					(
						-- Finally, compare again with benchmark file
						CompareWithBenchmark "EditNormalsSetSaveLoadTestBenchmark.max" ((c as string) + "c") 0.0 ($BoxObject).name true &vertexList &faceListBoxObject 1 false true
						CompareWithBenchmark "EditNormalsSetSaveLoadTestBenchmark.max" ((c as string) + "d") 0.0 ($PerforatedCylinder).name false &vertexList &faceListPerforatedCylinder 1 false true
					)

					-- Delete temporary saved file
					resetmaxFile #noPrompt
					deleteFile fileName
				)
			)
		)
	),

	Tests = #(EditNormalsSelectionTests,
	          EditNormalsUnifyBreakTests,
	          EditNormalsAveragingTests,
	          EditNormalsSetSaveLoadTests)
)

if (run_test_fixture == undefined) then 
( 
	format "-- No fixture!\n" 
	testList = EditNormalsUnitTests() 
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
	run_test_fixture EditNormalsUnitTests script:(getThisScriptFilename())
) 
