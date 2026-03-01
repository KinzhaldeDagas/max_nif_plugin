fileIn ((getFileNamePath(getSourceFileName())) + "/CompareWithBenchmark.mxs")

--------------------------------------------------------------------------------------
-- Tests for manipulation of geometry via the Bend modifier.
--
-- Author: Lee Betchen
-- Created: October 2021
--------------------------------------------------------------------------------------

Struct BendModUnitTests 
( 
	Function setup = 
	( 
	),

	Function teardown = 
	( 
	),

	Function BendModParameterTests = 
	(
		local axisX = 0
		local axisY = 1
		local axisZ = 2
		local nVerticesCompared = 100
		local nFacesCompared = 100
		local isPolyCase = #(true,false)

		-- Test the effect of each control parameter for the Bend modifier
		-- ===============================================================
		format "Bend Modifier -- Parameter Tests\n"

		-- With a relatively complex geometry, test the effect of each Bend modifier control parameter in two cases: after
		-- conversion to an Editable Poly, and to an Editable Mesh
		nCases = isPolyCase.count
		for c = 1 to nCases do
		(
			resetMaxFile #noprompt
			local caseName = c as string
			
			-- Load our test geometry
			local filePath = getFileNamePath(getSourceFileName())
			local fileName = filePath + "/TestData/BendModParameterTest.max"
			local isLoaded = loadMaxFile fileName quiet:true
			assert_true isLoaded message: ("Error Parameter Test " + caseName + " (load)")
			if (isLoaded) do
			(
				select $Cylinder001
				max modify mode

				-- Convert the test geometry to Editable Poly or Editable Mesh, as appropriate for case c
				local nVertices = $.numVerts
				local nFaces = $.numFaces
				if (isPolyCase[c]) then
				(
					convertTo $ PolyMeshObject
				)
				else
				(
					convertTo $ TriMeshGeometry
				)

				-- Build lists of vertex and face indices at which to compare the present results with our benchmarks
				local vertexList = #()
				vertexList.count = nVerticesCompared
				local vertexIncrement = nVertices / nVerticesCompared
				for v = 1 to nVerticesCompared do
				(
					vertexList[v] = v * vertexIncrement
				)

				local faceList = #()
				faceList.count = nFacesCompared
				local faceIncrement = nFaces / nFacesCompared
				for f = 1 to nFacesCompared do
				(
					faceList[f] = f * faceIncrement
				)

				-- Add a Bend modifier, setting initially for a bend along the x-axis
				addModifier $ (BendMod axis:axisX)

				-- Adjust the bend angle to 90 degrees, and compare against our benchmark result
				($.modifiers[1]).angle = 90.0
				CompareWithBenchmark "BendModParameterTestBenchmark1a.max" (caseName + "a") 0.0 "Cylinder001" true &vertexList &faceList 1 false false

				-- Switch to the y-axis and compare against our benchmark result, then repeat for the z-axis
				select $Cylinder001
				max modify mode
				($.modifiers[1]).axis = axisY
				CompareWithBenchmark "BendModParameterTestBenchmark1b.max" (caseName + "b") 0.0 "Cylinder001" true &vertexList &faceList 1 false false
				select $Cylinder001
				max modify mode
				($.modifiers[1]).axis = axisZ
				CompareWithBenchmark "BendModParameterTestBenchmark1c.max" (caseName + "c") 0.0 "Cylinder001" true &vertexList &faceList 1 false false

				-- Switch back to the x-axis, adjust the direction parameter, and compare against our benchmark result
				select $Cylinder001
				max modify mode
				($.modifiers[1]).axis = axisX
				($.modifiers[1]).direction = 10.0
				CompareWithBenchmark "BendModParameterTestBenchmark1d.max" (caseName + "d") 0.0 "Cylinder001" true &vertexList &faceList 1 false false

				-- Adjust the center of the Bend gizmo, and compare against our benchmark result
				select $Cylinder001
				max modify mode
				($.modifiers[1]).center += [0.0,0.0,-50.0]
				CompareWithBenchmark "BendModParameterTestBenchmark1e.max" (caseName + "e") 0.0 "Cylinder001" true &vertexList &faceList 1 false false

				-- Adjust the gizmo position directly through the gizmo itself, and compare against our benchmark result
				select $Cylinder001
				max modify mode
				(($.modifiers[1]).gizmo).position += [0.0,-50.0,0.0]
				CompareWithBenchmark "BendModParameterTestBenchmark1f.max" (caseName + "f") 0.0 "Cylinder001" true &vertexList &faceList 1 false false

				-- Rotate the gizmo with respect to its center, and compare against our benchmark result
				select $Cylinder001
				max modify mode
				(($.modifiers[1]).gizmo).rotation = Quat 0.0 0.0 1.0 1.0
				CompareWithBenchmark "BendModParameterTestBenchmark1g.max" (caseName + "g") 0.0 "Cylinder001" true &vertexList &faceList 1 false false

				-- Scale the gizmo with respect to its center, and compare against our benchmark result
				select $Cylinder001
				max modify mode
				(($.modifiers[1]).gizmo).scale = [2.0,2.0,2.0]
				CompareWithBenchmark "BendModParameterTestBenchmark1h.max" (caseName + "h") 0.0 "Cylinder001" true &vertexList &faceList 1 false false

				-- Apply limiting constraints, with the default lowere and upper limit values of 0.0, and compare against our
				-- benchmark result
				select $Cylinder001
				max modify mode
				($.modifiers[1]).limit = true
				CompareWithBenchmark "BendModParameterTestBenchmark1i.max" (caseName + "i") 0.0 "Cylinder001" true &vertexList &faceList 1 false false

				-- Adjust the upper limit, and compare against our benchmark result
				select $Cylinder001
				max modify mode
				($.modifiers[1]).upperlimit = 10.0
				CompareWithBenchmark "BendModParameterTestBenchmark1j.max" (caseName + "j") 0.0 "Cylinder001" true &vertexList &faceList 1 false false

				-- Adjust the upper limit, and compare against our benchmark result
				select $Cylinder001
				max modify mode
				($.modifiers[1]).lowerlimit = -10.0
				CompareWithBenchmark "BendModParameterTestBenchmark1k.max" (caseName + "k") 0.0 "Cylinder001" false &vertexList &faceList 1 false false
			)
		)
	),

	Function BendModStackTests =
	(
		local axisX = 0
		local nDim = 3
		local nCubesPair = 2
		local nVerticesCube = 8
		local nFacesCube = 6
		local heightInit = 50.0

		-- Test stack navigation, multi-modifier stacks, and modifier instancing with the Bend modifier
		-- ============================================================================================
		format "Bend Modifier -- Stack Tests\n"

		-- First test; create simple box, with a height equal to twice its width and length, and add a Chamfer modifier
		resetMaxFile #noprompt
		local boxCurr = Box lengthsegs:1 widthsegs:1 heightsegs:1 pos:[0.0,0.0,0.0] isSelected:on length:(0.5 * heightInit) width:(0.5 * heightInit) height:heightInit mapCoords:true
		select boxCurr
		max modify mode
		addModifier $ (ChamferMod segments:3 amount:5.0)

		-- Add one bend modifier along each axis
		for i = 1 to nDim do
		(
			addModifier $ (BendMod axis:(axisX + i - 1) angle:30.0)
		)
		
		-- Build lists of vertices and faces at which results should be compared, and compare against our benchmark result
		local vertexList = #()
		local nVertices = $.numVerts
		vertexList.count = nVertices
		for v = 1 to nVertices do
		(
			vertexList[v] = v
		)

		local faceList = #()
		local nFaces = $.numFaces
		faceList.count = nFaces
		for f = 1 to nFaces do
		(
			faceList[f] = f
		)

		CompareWithBenchmark "BendModStackTestBenchmark1a.max" "1a" 0.0 boxCurr.name true &vertexList &faceList 1 true false

		-- Navigate down to our base object and change the height to be double the initial height, then navigate back to the
		-- top of the stack and compare against our benchmark results
		select boxCurr
		max modify mode
		modPanel.setCurrentObject $.baseObject
		($.baseObject).height = 2.0 * heightInit
		modPanel.setCurrentObject $.modifiers[1]
		CompareWithBenchmark "BendModStackTestBenchmark1b.max" "1b" 0.0 boxCurr.name true &vertexList &faceList 1 true false

		-- Navigate down to our y-axis Bend modifier and deactivate it, then navigate back to the top of the stack and compare
		-- against our benchmark results
		select boxCurr
		max modify mode
		modPanel.setCurrentObject $.modifiers[2]
		($.modifiers[2]).enabled = false
		modPanel.setCurrentObject $.modifiers[1]
		CompareWithBenchmark "BendModStackTestBenchmark1c.max" "1c" 0.0 boxCurr.name false &vertexList &faceList 1 true false		

		-- Second test; create a pair of simple cubes, and add a shared bend modifier
		resetMaxFile #noprompt
		local cubes = #()
		cubes.count = nCubesPair
		cubes[1] = Box lengthsegs:1 widthsegs:1 heightsegs:1 pos:[0.0,0.0,0.0] isSelected:on length:50.0 width:50.0 height:50.0 mapCoords:true
		cubes[2] = Box lengthsegs:1 widthsegs:1 heightsegs:1 pos:[50.0,0.0,0.0] isSelected:on length:30.0 width:30.0 height:30.0 mapCoords:true
		select #(cubes[1], cubes[2])
		max modify mode
		modPanel.addModToSelection (BendMod()) ui:on
		((cubes[1]).modifiers[1]).axis = axisX
		((cubes[1]).modifiers[1]).angle = 90.0

		-- Add a shared Weighted Normals modifier
		modPanel.addModToSelection (WeightedNormalsMod()) ui:on

		-- Build lists of vertices and faces at which results should be compared, and compare against our benchmark result
		vertexList.count = nVerticesCube
		for v = 1 to nVerticesCube do
		(
			vertexList[v] = v
		)

		faceList.count = nFacesCube
		for f = 1 to nFacesCube do
		(
			faceList[f] = f
		)

		local cubePostfixes = #("a","b")
		for c = 1 to nCubesPair do
		(
			CompareWithBenchmark "BendModStackTestBenchmark2a.max" ("2" + cubePostfixes[c]) 0.0 (cubes[c]).name true &vertexList &faceList 1 false true
		)

		-- Navigate down to our Bend modifier and adjust the angle value, then navigate back to the top of the stack and
		-- compare against our benchmark results
		select cubes[1]
		max modify mode
		modPanel.setCurrentObject $.modifiers[2]
		($.modifiers[2]).angle = 45.0
		modPanel.setCurrentObject $.modifiers[1]
		select #(cubes[1], cubes[2])
		cubePostfixes = #("c","d")
		for c = 1 to nCubesPair do
		(
			CompareWithBenchmark "BendModStackTestBenchmark2c.max" ("2" + cubePostfixes[c]) 0.0 (cubes[c]).name true &vertexList &faceList 1 false true
		)
	),

	Function BendModCloneSaveLoadTests =
	(
		local axisX = 0
		local nVerticesCompared = 100
		local nFacesCompared = 100
		local isCloneCase = #(true, false)

		-- Test cloning, saving, and loading of an object with a Bend modifier
		-- ===================================================================
		format "Bend Modifier -- Clone and Save/Load Tests\n"

		-- Apply a Bend modifier to a relatively complex geometry, and test cloning and save/load
		nCases = isCloneCase.count
		for c = 1 to nCases do
		(
			resetMaxFile #noprompt
			local caseName = c as string
			
			-- Load our test geometry
			local filePath = getFileNamePath(getSourceFileName())
			local fileName = filePath + "/TestData/BendModParameterTest.max"
			local isLoaded = loadMaxFile fileName quiet:true
			assert_true isLoaded message: ("Error Clone and Save/Load Test " + caseName + " (load)")
			if (isLoaded) do
			(
				select $Cylinder001
				max modify mode

				-- Build lists of vertex and face indices at which to compare the present results with our benchmarks
				local vertexList = #()
				local nVertices = $.numVerts
				vertexList.count = nVerticesCompared
				local vertexIncrement = nVertices / nVerticesCompared
				for v = 1 to nVerticesCompared do
				(
					vertexList[v] = v * vertexIncrement
				)

				local faceList = #()
				local nFaces = $.numFaces
				faceList.count = nFacesCompared
				local faceIncrement = nFaces / nFacesCompared
				for f = 1 to nFacesCompared do
				(
					faceList[f] = f * faceIncrement
				)

				-- Add a Bend modifier with a bend angle of 90 degrees along the x-axis, and compare against our benchmark
				-- result
				addModifier $ (BendMod axis:axisX angle:90.0)
				CompareWithBenchmark "BendModParameterTestBenchmark1a.max" (caseName + "a") 0.0 "Cylinder001" true &vertexList &faceList 1 false false

				-- Proceed according to whether we are testing cloning, or save/load operations
				select $Cylinder001
				max modify mode
				if (isCloneCase[c]) then
				(
					-- We are testing cloning; we check above that the present results match our benchmark with cloning in
					-- CompareWithBenchmark, so to validate cloning functionality, simply compare again without cloning to
					-- confirm that we match the benchmark in both cases
					CompareWithBenchmark "BendModParameterTestBenchmark1a.max" (caseName + "b") 0.0 "Cylinder001" false &vertexList &faceList 1 false false
				)
				else
				(
					-- We are testing save/load operations; first, save the present model to a temporary file
					local isSaved = false
					filePath = GetDir #temp
					fileName = filePath + "/TempFile_BendModCloneSaveLoadTest.max"
					local isNameUnavailable = isMaxFile fileName
					if (not isNameUnavailable) do
					(
						isSaved = saveMaxFile fileName quiet:true
					)

					assert_true isSaved message: ("Error Clone and Save/Load Test " + (c as string) + " (save)")
					if (isSaved) do
					(
						-- Next, load the file
						resetmaxFile #noPrompt
						isLoaded = loadMaxFile fileName quite:true
						assert_true isLoaded message: ("Error Clone and Save/Load Test " + (c as string) + " (load from saved)")
						if (isLoaded) do
						(
							-- Finally, compare the saved file against our benchmark result
							CompareWithBenchmark "BendModParameterTestBenchmark1a.max" (caseName + "b") 0.0 "Cylinder001" false &vertexList &faceList 1 false false
						)

						-- Delete temporary saved file
						resetmaxFile #noPrompt
						deleteFile fileName
					)
				)
			)
		)
	),
	
	Function BendModAnimationTests = 
	(
		local axisZ = 2

		-- Test an animated bend via the Bend modifier
		-- ===========================================
		format "Bend Modifier -- Animation Tests\n"
		
		-- Build a simple box, and apply a Bend modifier along the z-axis
		resetMaxFile #noprompt
		local boxCurr = Box lengthsegs:1 widthsegs:1 heightsegs:1 pos:[0.0,0.0,0.0] isSelected:on width:50.0 length:50.0 height:100.0 mapCoords:true
		max modify mode
		select boxCurr
		addModifier $ (BendMod axis:axisZ angle:0.0)

		-- Animate the Bend modifier
		with animate on
		(
			at time 10 ($.modifiers[1]).angle = 90.0
		)

		-- Take a snapshot of the model halfway through the animation, build lists of vertices and faces at which results
		-- should be compared, and compare against our benchmark result
		sliderTime = 5
		snapshot boxCurr
		select $Box002
		convertTo $ PolyMeshObject
		move $ [100.0,0,0]
		local vertexList = #()
		local nVertices = $.numVerts
		vertexList.count = nVertices
		for v = 1 to nVertices do
		(
			vertexList[v] = v
		)

		local faceList = #()
		local nFaces = $.numFaces
		faceList.count = nFaces
		for f = 1 to nFaces do
		(
			faceList[f] = f
		)

		CompareWithBenchmark "BendModAnimationTestBenchmark1.max" "1" 0.0 "Box002" false &vertexList &faceList 1 true false
	),

	Tests = #(BendModParameterTests,
	          BendModStackTests,
	          BendModCloneSaveLoadTests,
	          BendModAnimationTests)
)

if (run_test_fixture == undefined) then 
( 
	format "-- No fixture!\n" 
	testList = BendModUnitTests()
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
	run_test_fixture BendModUnitTests script:(getThisScriptFilename())
) 
