fileIn ((getFileNamePath(getSourceFileName())) + "/CompareWithBenchmark.mxs")

--------------------------------------------------------------------------------------
-- Tests for mesh refinement via the Tessellate modifier.
--
-- Author: Lee Betchen
-- Created: January 2022
--------------------------------------------------------------------------------------

Struct TessellateUnitTests 
( 
	Function setup = 
	( 
	),

	Function teardown = 
	( 
	),

	-- Returns the number of polygon faces in the presently-selected object, in its state at the "top" of the modifier stack.
	Function GetNumberOfPolygonFaces =
	(
		-- Add an Edit Poly modifier, and check the number of faces
		local objCurr = modPanel.getCurrentObject()
		addModifier $ (Edit_Poly())
		local nFaces = ($.modifiers[1]).GetNumFaces()

		-- Delete the Edit Poly modifier, and revert the Modify panel selection to its state on entry
		deleteModifier $ 1
		modPanel.setCurrentObject objCurr

		-- Return the number of faces
		nFaces
	),

	-- Build an array of evenly-spaced sampling values over the integer range [minValue, maxValue]. In particular, entry
	-- sampleList[val] of the sampling array, for val on the range [1, nVals], is given by:
	-- 
	--     sampleList[val] = minValue + (val - 1) * deltaVal
	--
	-- If nValuesRange >= nValues, where nValuesRange = maxValue - minValue + 1, then nVals = nValues and
	-- deltaVal = nValuesRange / nValues, otherwise, nVals = nValuesRange and deltaVal = 1
	--
	-- sampleList:         sampling array, as detailed in the function description; output
	-- minValue, maxValue: bounding values of the integer value range [minValue, maxValue] to be sampled; input
	-- nValues:            desired number of sampling values; output
	Function BuildSamplingArray &sampleList minValue maxValue nValues =
	(
		sampleList.count = 0
		if ((maxValue >= minValue) and (nValues > 0)) do
		(
			local nVals = nValues
			local nValuesRange = maxValue - minValue + 1
			local deltaVal = nValuesRange / nValues
			if (deltaVal < 1) do
			(
				nVals = nValuesRange
				deltaVal = 1
			)

			sampleList.count = nVals
			for val = 1 to nVals do
			(
				sampleList[val] = minValue + (val - 1) * deltaVal
			)
		)
	),

	Function TessellateParameterTests = 
	(
		local nIterationsMax = 3
		local nFacesCompared = 100
		local isPolyCase = #(true,false)
		local doCompareBeforeRender = #(true,false)

		-- Test the effect of each control parameter for the Tessellate modifier
		-- =====================================================================
		format "Tessellate Modifier -- Parameter Tests\n"

		-- First and second tests; with a relatively complex geometry, test the effect of each Tessellate modifier control
		-- parameter affecting the actual algorithm in two cases: after conversion to an Editable Poly, and to an Editable Mesh
		local vertexList = #()
		local faceList = #()
		local nCases = isPolyCase.count
		local filePath = getFileNamePath(getSourceFileName())
		local fileName = filePath + "/TestData/TessellateParameterTest.max"
		for c = 1 to nCases do
		(
			resetMaxFile #noprompt
			local caseName = c as string
			
			-- Load our test geometry
			local isLoaded = loadMaxFile fileName quiet:true
			assert_true isLoaded message: ("Error Parameter Test " + caseName + " (load)")
			if (isLoaded) do
			(
				select $1_original
				max modify mode

				-- Convert the test geometry to Editable Poly or Editable Mesh, as appropriate for case c
				if (isPolyCase[c]) then
				(
					convertTo $ PolyMeshObject
				)
				else
				(
					convertTo $ TriMeshGeometry
				)

				-- Add a Tessellate modifier, setting initially to operate on polygonal faces, with zero tension, a single
				-- iteration, and automatic updates when parameters are adjusted, and compare against our benchmark result
				addModifier $ (Tessellate updateWhen:0 faceType:1 iterations:0 tension:0.0)
				local nFaces = GetNumberOfPolygonFaces()
				BuildSamplingArray &faceList 1 nFaces nFacesCompared
				CompareWithBenchmark ("TessellateParameterTestBenchmark" + caseName + "a.max") (caseName + "a") 0.0 "1_original" true &vertexList &faceList 1 true true

				-- Set a positive tension, and compare against our benchmark result
				select $1_original
				max modify mode
				($.modifiers[1]).tension = 1000.0
				nFaces = GetNumberOfPolygonFaces()
				BuildSamplingArray &faceList 1 nFaces nFacesCompared
				CompareWithBenchmark ("TessellateParameterTestBenchmark" + caseName + "b.max") (caseName + "b") 0.0 "1_original" true &vertexList &faceList 1 true true

				-- Set a negative tension, and compare against our benchmark result
				select $1_original
				max modify mode
				($.modifiers[1]).tension = -1000.0
				nFaces = GetNumberOfPolygonFaces()
				BuildSamplingArray &faceList 1 nFaces nFacesCompared
				CompareWithBenchmark ("TessellateParameterTestBenchmark" + caseName + "c.max") (caseName + "c") 0.0 "1_original" true &vertexList &faceList 1 true true

				-- Reset to zero tension, and recompute for each available iteration count, comparing each result against our
				-- benchmark
				local refinementCases = #("d","e","f")
				select $1_original
				max modify mode
				($.modifiers[1]).tension = 0.0
				for i = 1 to nIterationsMax do
				(
					select $1_original
					max modify mode
					($.modifiers[1]).iterations = i
					nFaces = GetNumberOfPolygonFaces()
					BuildSamplingArray &faceList 1 nFaces nFacesCompared
					CompareWithBenchmark ("TessellateParameterTestBenchmark" + caseName + refinementCases[i] + ".max") (caseName + refinementCases[i]) 0.0 "1_original" true &vertexList &faceList 1 true true
				)

				-- Reset to a single iteration, recompute with new vertices added at face centers rather than edge midpoints,
				-- and compare against our benchmark result
				select $1_original
				max modify mode
				($.modifiers[1]).iterations = 0
				($.modifiers[1]).type = 1
				nFaces = GetNumberOfPolygonFaces()
				BuildSamplingArray &faceList 1 nFaces nFacesCompared
				CompareWithBenchmark ("TessellateParameterTestBenchmark" + caseName + "g.max") (caseName + "g") 0.0 "1_original" true &vertexList &faceList 1 true true

				-- Reset to add new vertices at edge midpoints, set to operate on face triangulations, rather than polygon
				-- faces, recompute, and compare against our benchmark result
				select $1_original
				max modify mode
				($.modifiers[1]).type = 0
				($.modifiers[1]).faceType = 0
				nFaces = GetNumberOfPolygonFaces()
				BuildSamplingArray &faceList 1 nFaces nFacesCompared
				CompareWithBenchmark ("TessellateParameterTestBenchmark" + caseName + "h.max") (caseName + "h") 0.0 "1_original" false &vertexList &faceList 1 true true
			)
		)

		-- Third test; test update on render option
		nCases = doCompareBeforeRender.count
		for c = 1 to nCases do
		(
			-- Create a simple box, add a Tessellate modifier, setting initially to operate on polygonal faces, with zero
			-- tension, a single iteration, and updates only on render, and then adjust to the maximum number of iterations
			resetMaxFile #noprompt
			local boxCurr = Box lengthsegs:1 widthsegs:1 heightsegs:1 pos:[0.0,0.0,0.0] isSelected:on length:50.0 width:50.0 height:50.0 mapCoords:true
			select boxCurr
			max modify mode
			addModifier $ (Tessellate updateWhen:1 faceType:1 iterations:0 tension:0.0)
			($.modifiers[1]).iterations = nIterationsMax

			-- Proceed according to whether we are to compare before or after rendering in case c; note that because the
			-- Tessellate modifier does not cache its output mesh, cloning an object with a Tessellate modifier in on-render
			-- update mode does not properly preserve the geometry, so that we must compare without cloning our box, hence the
			-- need to build the object twice in order to check results before and after rendering
			if (doCompareBeforeRender[c]) then
			(
				-- Compare against our benchmark result without rendering; note that because we set the Tessellate modifier to
				-- update only on render, the output mesh should at this juncture be consistent with the initial setting of one
				-- iteration 
				local nFacesCurr = GetNumberOfPolygonFaces()
				BuildSamplingArray &faceList 1 nFacesCurr nFacesCompared
				CompareWithBenchmark "TessellateParameterTestBenchmark3a.max" "3a" 0.0 boxCurr.name false &vertexList &faceList 1 true true
			)
			else
			(
				-- Perform a render to trigger an update of the output mesh, and compare against our benchmark result
				render()
				select boxCurr
				max modify mode
				nFacesCurr = GetNumberOfPolygonFaces()
				BuildSamplingArray &faceList 1 nFacesCurr nFacesCompared
				CompareWithBenchmark "TessellateParameterTestBenchmark3b.max" "3b" 0.0 boxCurr.name false &vertexList &faceList 1 true true
			)
		)
	),

	Function TessellateStackTests =
	(
		local nIterationsMax = 3
		local nFacesCompared = 100
		local boxDimensions = #(50.0,100.0)
		local boxOffsetCoeffs = #(-1.0,1.0)

		-- Test stack navigation, multi-modifier stacks, and modifier instancing with the Tessellate modifier
		-- ==================================================================================================
		format "Tessellate Modifier -- Stack Tests\n"

		-- First test; load a file containing a relatively complex geometry
		resetMaxFile #noprompt
		local vertexList = #()
		local faceList = #()
		local filePath = getFileNamePath(getSourceFileName())
		local fileName = filePath + "/TestData/TessellateParameterTest.max"
		local isLoaded = loadMaxFile fileName quiet:true
		assert_true isLoaded message: ("Error Stack Test 1 (load)")
		if (isLoaded) do
		(
			select $1_original
			max modify mode

			-- Add a Tesselate modifier, with a weighted normals modifier above it, and compare against our benchmark result;
			-- note that we compare vertex normals as well as mesh geometry and topology
			addModifier $ (Tessellate updateWhen:0 faceType:1 iterations:0 tension:0.0)
			addModifier $ (WeightedNormalsMod useHardEdgeAngle:true smoothingCoeff:1.0 boundaryCoeff:1.0)
			local nFaces = GetNumberOfPolygonFaces()
			BuildSamplingArray &faceList 1 nFaces nFacesCompared
			CompareWithBenchmark "TessellateStackTestBenchmark1a.max" "1a" 0.0 "1_original" true &vertexList &faceList 1 true true

			-- Navigate down to the underlying Editable Poly, switch to face subobject mode, and set a face selection, so that
			-- the Tesselate modifer will now operate only on those faces, then navigate back to the top of the stack, and
			-- compare against our benchmark result
			select $1_original
			max modify mode
			modPanel.setCurrentObject $.baseObject
			subobjectLevel = 4
			($.EditablePoly).SetSelection #Face #{615,616,617,618,619,620,621,622,623,624,625,626,627,628,629,630,631,632,633,634,635,636,637,638}
			modPanel.setCurrentObject $.modifiers[1]
			nFaces = GetNumberOfPolygonFaces()
			BuildSamplingArray &faceList 1 nFaces nFaces
			CompareWithBenchmark "TessellateStackTestBenchmark1b.max" "1b" 0.0 "1_original" false &vertexList &faceList 1 true true
		)

		-- Second test; create two simple boxes of differing sizes, and select both
		resetMaxFile #noprompt
		local nBoxes = boxDimensions.count
		local boxes = #()
		boxes.count = nBoxes
		for b = 1 to nBoxes do
		(
			xOffset = (boxOffsetCoeffs[b]) * (boxDimensions[b])
			boxes[b] = Box lengthsegs:1 widthsegs:1 heightsegs:1 pos:[xOffset,0.0,0.0] isSelected:on length:boxDimensions[b] width:boxDimensions[b] height:boxDimensions[b] mapCoords:true
		)

		select #(boxes[1], boxes[2])
		max modify mode

		-- Apply shared Chamfer, Tessellate, and Weighted Normals modifiers to our pair of boxes
		modPanel.addModToSelection (ChamferMod()) ui:on
		((boxes[1]).modifiers[1]).amount = 5.0
		modPanel.addModToSelection (Tessellate()) ui:on
		((boxes[1]).modifiers[1]).faceType = 1
		((boxes[1]).modifiers[1]).tension = 0.0
		modPanel.addModToSelection (WeightedNormalsMod()) ui:on
		((boxes[1]).modifiers[1]).useHardEdgeAngle = true
		((boxes[1]).modifiers[1]).smoothingCoeff = 1.0
		((boxes[1]).modifiers[1]).boundaryCoeff = 1.0

		-- Compare each box against our benchmark result; note that we compare vertex normals as well as mesh geometry and
		-- topology
		for b = 1 to nBoxes do
		(
			select boxes[b]
			max modify mode
			local nFaces = GetNumberOfPolygonFaces()
			BuildSamplingArray &faceList 1 nFaces nFacesCompared
			CompareWithBenchmark "TessellateStackTestBenchmark2a.max" "2a" 0.0 (boxes[b]).name true &vertexList &faceList 1 true true
		)

		-- Adjust the chamfer amount, and the number of iterations of refinement for the Tessellate modifier
		((boxes[1]).modifiers[3]).amount = 10.0
		((boxes[1]).modifiers[2]).iterations = nIterationsMax

		-- Compare each box against our benchmark result
		for b = 1 to nBoxes do
		(
			select boxes[b]
			max modify mode
			local nFaces = GetNumberOfPolygonFaces()
			BuildSamplingArray &faceList 1 nFaces nFacesCompared
			CompareWithBenchmark "TessellateStackTestBenchmark2b.max" "2b" 0.0 (boxes[b]).name true &vertexList &faceList 1 true true
		)
	),

	Function TessellateCloneSaveLoadTests =
	(
		local nFacesCompared = 100
		local isCloneCase = #(true, false)

		-- Test cloning, saving, and loading of an object with a Tessellate modifier
		-- =========================================================================
		format "Tessellate Modifier -- Clone and Save/Load Tests\n"

		-- Apply a Tessellate modifier to a relatively complex geometry, and test cloning and save/load
		local vertexList = #()
		local faceList = #()
		local nCases = isCloneCase.count
		for c = 1 to nCases do
		(
			resetMaxFile #noprompt
			local caseName = c as string
			
			-- Load our test geometry
			local filePath = getFileNamePath(getSourceFileName())
			local fileName = filePath + "/TestData/TessellateParameterTest.max"
			local isLoaded = loadMaxFile fileName quiet:true
			assert_true isLoaded message: ("Error Clone and Save/Load Test " + caseName + " (load)")
			if (isLoaded) do
			(
				select $1_original
				max modify mode

				-- Add a Tessellate modifier, setting to operate on polygonal faces, with zero tension, a single iteration, and
				-- automatic updates when parameters are adjusted, and compare against our benchmark result
				addModifier $ (Tessellate updateWhen:0 faceType:1 iterations:0 tension:0.0)
				local nFaces = GetNumberOfPolygonFaces()
				BuildSamplingArray &faceList 1 nFaces nFacesCompared
				CompareWithBenchmark "TessellateParameterTestBenchmark1a.max" (caseName + "a") 0.0 "1_original" true &vertexList &faceList 1 true true

				-- Proceed according to whether we are testing cloning, or save/load operations
				select $1_original
				max modify mode
				if (isCloneCase[c]) then
				(
					-- We are testing cloning; we check above that the present results match our benchmark with cloning in
					-- CompareWithBenchmark, so to validate cloning functionality, simply compare again without cloning to
					-- confirm that we match the benchmark in both cases
					CompareWithBenchmark "TessellateParameterTestBenchmark1a.max" (caseName + "b") 0.0 "1_original" false &vertexList &faceList 1 true true
				)
				else
				(
					-- We are testing save/load operations; first, save the present model to a temporary file
					local isSaved = false
					filePath = GetDir #temp
					fileName = filePath + "/TempFile_TessellateCloneSaveLoadTest.max"
					local isNameUnavailable = isMaxFile fileName
					if (not isNameUnavailable) do
					(
						isSaved = saveMaxFile fileName quiet:true
					)

					assert_true isSaved message: ("Error Clone and Save/Load Test " + (c as string) + " (save)")
					if (isSaved) do
					(
						-- Next, load the saved file
						resetmaxFile #noPrompt
						isLoaded = loadMaxFile fileName quite:true
						SceneConverter.visible = false
						assert_true isLoaded message: ("Error Clone and Save/Load Test " + (c as string) + " (load from saved)")
						if (isLoaded) do
						(
							-- Finally, compare the saved file against our benchmark result
							CompareWithBenchmark "TessellateParameterTestBenchmark1a.max" (caseName + "b") 0.0 "1_original" false &vertexList &faceList 1 true true
						)

						-- Delete temporary saved file
						resetmaxFile #noPrompt
						deleteFile fileName
					)
				)
			)
		)
	),

	Function TessellateAnimationTests = 
	(
		local axisZ = 2
		local nFacesCompared = 100
		
		-- Test animation with the Tessellate modifier
		-- ===========================================
		format "Tessellate Modifier -- Animation Tests\n"
		
		-- We want to test both animation of parameters of the Tessellate modifier, as well as propagation of animation
		-- below the modifier on the stack; build a simple hemisphere, and apply a Bend modifier
		resetMaxFile #noprompt
		local sphereCurr = Sphere segs:32 pos:[0.0,0.0,0] isSelected:on radius:25.0 hemisphere:0.5
		select sphereCurr
		max modify mode
		addModifier $ (BendMod axis:axisZ angle:0.0)

		-- Animate the Bend modifier
		with animate on
		(
			at time 10 ($.modifiers[1]).angle = 45.0
		)

		-- Add a Tessellate modifier, setting to operate on polygonal faces, with zero tension, a single iteration, and
		-- automatic updates when parameters are adjusted, and compare against our benchmark result
		addModifier $ (Tessellate updateWhen:0 faceType:1 iterations:0 tension:0.0)

		-- Animate the Tessellate modifier
		with animate on
		(
			at time 10 ($.modifiers[1]).tension = 10000.0
		)

		-- Add a Weighted Normals modifier
		addModifier $ (WeightedNormalsMod useHardEdgeAngle:true smoothingCoeff:1.0 boundaryCoeff:1.0)

		-- Take a snapshot of the model halfway through the animation, and compare against our benchmark result
		sliderTime = 5
		snapshot sphereCurr
		select $Sphere002
		move $ [100.0,0,0]
		vertexList = #()
		faceList = #()
		local nFaces = GetNumberOfPolygonFaces()
		BuildSamplingArray &faceList 1 nFaces nFacesCompared
		CompareWithBenchmark "TessellateAnimationTestBenchmark1.max" "1" 0.0 "Sphere002" false &vertexList &faceList 1 true true
	),

	Tests = #(TessellateParameterTests,
	          TessellateStackTests,
	          TessellateCloneSaveLoadTests,
	          TessellateAnimationTests)
)

if (run_test_fixture == undefined) then 
( 
	format "-- No fixture!\n" 
	testList = TessellateUnitTests()
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
	run_test_fixture TessellateUnitTests script:(getThisScriptFilename())
) 
