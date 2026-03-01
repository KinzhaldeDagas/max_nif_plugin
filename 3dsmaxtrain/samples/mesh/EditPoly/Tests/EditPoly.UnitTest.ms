/*
	@param TestName: EditPoly
	@param Author:   Peter Watje
	@param Created:  4/1/2013
	@param LastMod:  02/13/2024
	@param ModBy:    Larry Minton
	@param Desc:     Edit Poly modifier's Unit tests
	@param Notes:
	@param Status:   Online
*/


fileIn "MxsUnit/MxsUnit.ms"
fileIn "CompareWithBenchmark.mxs"

struct Obj_Prop_UnitTest
(
private


	--##################################################
	-- Helper functions
	--##################################################


	function validateProperties experimental expected =
	(
		if (experimental.count != expected.count) then
		(
			assert_equal expected.count experimental.count message:"Incorrect number of properties"
		)
		else
		(
			for i in 1 to experimental.count do
			(
				assert_equal expected[i] experimental[i] message:("Incorrect Value for " + testprops[i])
			)
		)
	),

	function changeValue valueName pvalue properties =
	(
		for i in 1 to properties.count do
		(
			if testprops[i] == valueName do
			(
				properties[i] = pvalue
				return true
			)
		)
		local str = stringstream ""
		format "Value(%): %, not found in property array" valueName pvalue to:str
		assert_true false message:(str as string)
	),
	
	function run_max file logs = 
	(
	
		mb = pathConfig.appendPath (getDir #maxroot) "3dsmax.exe"
		cmd = "\"\"" + mb + "\" -u MAXScript \"" + file + "\" -silent -listenerlog \"" + logs + "\"\""
		format "Logs %\n" logs
		format "Command %\n" cmd
		res = DOSCommand cmd
		assert_equal 0 res message:"The dos command returned the valid 0 result"
	),
	
	function findTextInFile fname textToFind =
	(
		foundIt = false
		local logFile = openfile fname
		while not eof logFile and not foundIt do
		(
			local fileLine = readLine logFile
			if (findString fileLine textToFind) != undefined do
			(
				foundIt = true
			)
		)
		close logFile
		
		foundIt -- return value
	),
	
	-- If a scene object is selected, converts the object to an Editable Poly with exclusively triangular faces, matching the
	-- stored triangulation of the object on entry. This is accomplished by converting to an Editable Mesh, setting all face
	-- edges visible, and then converting to an Editable Poly.
	function TriangulateSelectedObject =
	(
		local nEdgesTriangle = 3

		if ($ != undefined) do
		(
			max modify mode
			convertTo $ TriMeshGeometry
			local nTriangles = $.numFaces
			for f = 1 to nTriangles do
			(
				for edge = 1 to nEdgesTriangle do
				(
					setEdgeVis $ f edge true
				)
			)

			update $
			convertTo $ PolyMeshObject
		)
	),

	-- If a scene object is selected, converts the object to an Editable Poly with exclusively triangular faces, matching the
	-- stored triangulation of the object on entry. This is accomplished by converting to an Editable mesh, setting all face
	-- edges visible, and then converting to an Editable Poly. Additionally, forms a list of triangles in the triangulated mesh
	-- corresponding to the input mesh faces specified in faceList. In order to form this list, the face materials are reset,
	-- with all faces in faceList sharing one material index, and all faces not in that list sharing another.
	-- 
	-- triangleList: list of triangles on the triangulated output mesh corresponding to input mesh faces specified in faceList;
	--               output
	-- faceList:     list of faces, in the selected object upon conversion to Editable Poly, for which corresponding triangles
	--               on the triangulated ouput mesh are to be determined; input
	function TriangulateAndBuildFaceTriangleList &triangleList &faceList =
	(
		local flaggedMatID = 2
		local unflaggedMatID = 1

		-- Proceed only if an object is selected
		if ($ != undefined) do
		(
			max modify mode
			convertTo $ PolyMeshObject

			-- Flag the specified faces with a different material ID than all other faces
			local nFaces = $.numFaces
			local allFaces = #()
			allFaces.count = nFaces
			for f = 1 to nFaces do
			(
				allFaces[f] = f
			)

			polyop.setFaceMatID $ allFaces unflaggedMatID
			polyop.setFaceMatID $ faceList flaggedMatID

			-- Split the mesh of the selected object into its triangulation
			TriangulateSelectedObject()

			-- Build a list of all faces with the material ID assigned to the specified faces, which will be the triangles of
			-- the triangulation of those faces
			triangleList.count = 0
			local nTriangles = $.numFaces
			for f = 1 to nTriangles do
			(
				local faceMaterialID = polyop.getFaceMatID $ f
				if (faceMaterialID == flaggedMatID) do
				(
					append triangleList f
				)
			)
		)
	),
	
	--##################################################
	-- Unit tests
	--##################################################
public
	function setup =
	(
	),

	function teardown =
	(

	),
/*
	Regression test for  MAXX_10215
	Max script index issue
	Fix : the code as thinking the index was 1 based, set it to zero base
*/
	function editPoly_MAXX_10215 =
	(

		Sphere radius:28.0464 smooth:on segs:32 chop:0 slice:off sliceFrom:0 sliceTo:0 mapcoords:on recenter:off transform:(matrix3 [1,0,0] [0,0,1] [0,-1,0] [-60.5568,1.08518e-006,-24.826]) isSelected:on
		max modify mode
		modPanel.addModToSelection (Uvwmap ()) ui:on
		$.modifiers[#UVW_Map].mapChannel = 4
		max create mode
		Sphere radius:28.8749 smooth:on segs:32 chop:0 slice:off sliceFrom:0 sliceTo:0 mapcoords:on recenter:off transform:(matrix3 [1,0,0] [0,0,1] [0,-1,0] [42.4594,1.34887e-006,-30.8585]) isSelected:on
		select #($Sphere002, $Sphere001)
		macros.run "Modifier Stack" "Convert_to_Poly"
		select $Sphere002
		modPanel.addModToSelection (Edit_Poly ()) ui:on
		$.modifiers[#Edit_Poly].Attach $Sphere001 editPolyNode:$


	),

	function editPoly_MAXX_12834_01 =
	(
		Plane length:329.895 width:539.251 transform:(matrix3 [1,0,0] [0,0,1] [0,-1,0] [-327.807,-2.96242e-006,67.7722]) isSelected:on lengthsegs:2 widthsegs:2
		max modify mode
		macros.run "Modifier Stack" "Convert_to_Poly"
		subobjectLevel = 4
		$.EditablePoly.SetSelection #Face #{4}
		$.EditablePoly.SetSelection #Face #{4}
		$.preserveUVs = on
		$.EditablePoly.SetSelection #Face #{4}
		$.outlineAmount = -34.8927
		$.EditablePoly.Outline ()
		$.EditablePoly.SetSelection #Face #{1}
		$.insetAmount = 51.9074
		$.EditablePoly.buttonOp #inset
		subobjectLevel = 0

		epsilon = 0.00001

		tmesh = snapshotAsMesh selection[1]

		meshVal = gettVert tmesh 1
		correctVal = [0,0,0]
		l = Length(meshVal-correctVal)
		assert_true (l < epsilon)
		meshVal = gettVert tmesh 2
		correctVal = [0.5,0,0]
		l = Length(meshVal-correctVal)
		assert_true (l < epsilon)
		meshVal = gettVert tmesh 3
		correctVal = [1,0,0]
		l = Length(meshVal-correctVal)
		assert_true (l < epsilon)
		meshVal = gettVert tmesh 4
		correctVal = [0,0.5,0]
		l = Length(meshVal-correctVal)
		assert_true (l < epsilon)
		meshVal = gettVert tmesh 5
		correctVal = [0.564706,0.605769,0]
		l = Length(meshVal-correctVal)
		assert_true (l < epsilon)
		meshVal = gettVert tmesh 6
		correctVal = [0.935294,0.605769,0]
		l = Length(meshVal-correctVal)
		assert_true (l < epsilon)
		meshVal = gettVert tmesh 7
		correctVal = [0,1,0]
		l = Length(meshVal-correctVal)
		assert_true (l < epsilon)
		meshVal = gettVert tmesh 8
		correctVal = [0.564706,0.894231,0]
		l = Length(meshVal-correctVal)
		assert_true (l < epsilon)
		meshVal = gettVert tmesh 9
		correctVal = [0.935294,0.894231,0]
		l = Length(meshVal-correctVal)
		assert_true (l < epsilon)
		meshVal = gettVert tmesh 10
		correctVal = [0.0962583,0.359654,0]
		l = Length(meshVal-correctVal)
		assert_true (l < epsilon)
		meshVal = gettVert tmesh 11
		correctVal = [0.0962583,0.157345,0]
		l = Length(meshVal-correctVal)
		assert_true (l < epsilon)
		meshVal = gettVert tmesh 12
		correctVal = [0.417304,0.140601,0]
		l = Length(meshVal-correctVal)
		assert_true (l < epsilon)
		meshVal = gettVert tmesh 13
		correctVal = [0.465434,0.428801,0]
		l = Length(meshVal-correctVal)
		assert_true (l < epsilon)

	),
	function editPoly_MAXX_12834_02 =
	(
		max create mode
		Plane length:527.85 width:717.757 transform:(matrix3 [1,0,0] [0,0,1] [0,-1,0] [-170.234,-3.91671e-006,89.6038]) isSelected:on
		max modify mode
		modPanel.addModToSelection (Edit_Poly ()) ui:on
		subobjectLevel = 4
		$.modifiers[#Edit_Poly].SetSelection #Face #{}
		$.modifiers[#Edit_Poly].Select #Face #{4}
		$.modifiers[#Edit_Poly].preserveUVs = on
		$.modifiers[#Edit_Poly].SetOperation #Outline
		$.modifiers[#Edit_Poly].outlineAmount = -68.785
		$.modifiers[#Edit_Poly].Commit ()
		$.modifiers[#Edit_Poly].SetSelection #Face #{}
		$.modifiers[#Edit_Poly].Select #Face #{1}
		$.modifiers[#Edit_Poly].SetOperation #Inset
		$.modifiers[#Edit_Poly].insetAmount = 76.2617
		$.modifiers[#Edit_Poly].Commit ()


		epsilon = 0.00001

		tmesh = snapshotAsMesh selection[1]

		meshVal = gettVert tmesh 1
		correctVal = [0,0,0]
		l = Length(meshVal-correctVal)
		assert_true (l < epsilon)
		meshVal = gettVert tmesh 2
		correctVal = [0.5,0,0]
		l = Length(meshVal-correctVal)
		assert_true (l < epsilon)
		meshVal = gettVert tmesh 3
		correctVal = [1,0,0]
		l = Length(meshVal-correctVal)
		assert_true (l < epsilon)
		meshVal = gettVert tmesh 4
		correctVal = [0,0.5,0]
		l = Length(meshVal-correctVal)
		assert_true (l < epsilon)
		meshVal = gettVert tmesh 5
		correctVal = [0.595833,0.630312,0]
		l = Length(meshVal-correctVal)
		assert_true (l < epsilon)
		meshVal = gettVert tmesh 6
		correctVal = [0.904167,0.630312,0]
		l = Length(meshVal-correctVal)
		assert_true (l < epsilon)
		meshVal = gettVert tmesh 7
		correctVal = [0,1,0]
		l = Length(meshVal-correctVal)
		assert_true (l < epsilon)
		meshVal = gettVert tmesh 8
		correctVal = [0.595833,0.869688,0]
		l = Length(meshVal-correctVal)
		assert_true (l < epsilon)
		meshVal = gettVert tmesh 9
		correctVal = [0.904167,0.869688,0]
		l = Length(meshVal-correctVal)
		assert_true (l < epsilon)
		meshVal = gettVert tmesh 10
		correctVal = [0.10625,0.376904,0]
		l = Length(meshVal-correctVal)
		assert_true (l < epsilon)
		meshVal = gettVert tmesh 11
		correctVal = [0.10625,0.144476,0]
		l = Length(meshVal-correctVal)
		assert_true (l < epsilon)
		meshVal = gettVert tmesh 12
		correctVal = [0.411473,0.131349,0]
		l = Length(meshVal-correctVal)
		assert_true (l < epsilon)
		meshVal = gettVert tmesh 13
		correctVal = [0.474261,0.45739,0]
		l = Length(meshVal-correctVal)
		assert_true (l < epsilon)


	),

	function EditPolyNormalTest =
	(
		local nDim = 3
		local nBoxesAttachTest = 2

		-- Test preservation and adjustment of specified normals through some edit poly operations
		format "Edit Poly specified normals tests\n"

		-- Test rotation of normals through Hinge From Edit operation
		resetMaxFile #noprompt
		filePath = getFileNamePath (getSourceFileName())
		fileName = filePath + "/TestData/EditPolyNormalTest.max"
		res = loadMaxFile fileName quiet:true
		assert_true res message: "Error loading file - Edit Poly Normal Test 1"
		select $Box003
		max modify mode
		addModifier $ (Edit_Poly())
		subObjectLevel = 4
		$.modifiers[1].SetSelection #Face #{}
		$.modifiers[1].Select #Face #{3}
		$.modifiers[1].SetOperation #HingeFromEdge
		$.modifiers[1].hingeAngle = 45
		$.modifiers[1].Commit()
		addModifier $ (Edit_Normals())
		nVertices = $.modifiers[1].GetFaceDegree 3
		Assert_equal 4 nVertices message: "Error in face degree - Edit Poly Normal Test 1"
		for i = 1 to nVertices do
		(
			indexNormal = $.modifiers[1].GetNormalID 3 i
			Assert_true ($.modifiers[1].GetNormalExplicit indexNormal) message: "Error in normal explicit flag - Edit Poly Normal Test 1"
			Assert_point3_equal [0.0, -0.707, -0.707] ($.modifiers[1].GetNormal indexNormal) tolerance:0.001 message: "Error in normal vector - Edit Poly Normal Test 1"
		)

		-- Test edge and face split and merge operations
		resetMaxFile #noprompt
		filePath = getFileNamePath (getSourceFileName())
		fileName = filePath + "/TestData/EditPolyNormalTest.max"
		res = loadMaxFile fileName quiet:true
		assert_true res message: "Error loading file - Edit Poly Normal Test 2"
		select $Box003
		max modify mode
		addModifier $ (Edit_Poly())
		subObjectLevel = 2
		$.modifiers[1].SetSelection #Edge #{}
		$.modifiers[1].Select #Edge #{41}
		$.modifiers[1].ButtonOp #RemoveEdge
		$.modifiers[1].SetOperation #InsertVertexInEdge
		$.modifiers[1].DivideEdge 5 0.5
		$.modifiers[1].DivideEdge 40 0.5
		$.modifiers[1].Commit()
		subobjectLevel = 1
		$.modifiers[1].SetSelection #Vertex #{}
		$.modifiers[1].Select #Vertex #{25}
		$.modifiers[1].Select #Vertex #{26}
		$.modifiers[#Edit_Poly].ButtonOp #ConnectVertices
		addModifier $ (Edit_Normals())
		faceIndices = #(18,26)
		pts = #([0.0,0.0,1.0],[0.0,-1.0,0.0],[0.0,-1.0,0.0],[0.0,-0.928,-0.373],[0.0,-0.7,0.714],[0.0,-0.928,-0.373],[0.0,-1.0,0.0],[0.0,-1.0,0.0],[0.0,0.0,1.0],[0.0,-0.7,0.714])
		for i = 1 to 2 do
		(
			nVertices = $.modifiers[1].GetFaceDegree faceIndices[i]
			Assert_equal 5 nVertices message: "Error in face degree - Edit Poly Normal Test 2"
			for j = 1 to nVertices do
			(
				indexNormal = $.modifiers[1].GetNormalID faceIndices[i] j
				Assert_point3_equal pts[(i - 1) * nVertices + j] ($.modifiers[1].GetNormal indexNormal) tolerance:0.001 message: "Error in normal vector - Edit Poly Normal Test 2"
			)
		)

		-- Test an attach operation between objects with applied transformations, to ensure that normals are transformed
		-- properly; create two boxes, and generate explicit normals on the second with the Weighted Normals modifier
		resetMaxFile #noprompt
		boxes = #()
		boxes.count = nBoxesAttachTest
		for b = 1 to nBoxesAttachTest do
		(
			xPosition = 100.0 * (b - 1)
			boxes[b] = Box lengthsegs:1 widthsegs:1 heightsegs:1 pos:[xPosition,0.0,0.0] isSelected:on length:50.0 width:50.0 height:50.0 mapCoords:true
			convertTo $ PolyMeshObject
		)

		select boxes[2]
		max modify mode
		addModifier $ (WeightedNormalsMod())
		convertTo $ PolyMeshObject

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

		-- Add an Edit Poly modifier to the first box, and attach the second
		select boxes[1]
		addModifier $ (Edit_Poly())
		($.modifiers[1]).SetOperation #Attach
		($.modifiers[1]).Attach $Box002 editPolyNode:$

		-- Check vertex normals over a selection of faces on the second box
		addModifier $ (Edit_Normals())
		faceIndices = #(8,9,12)
		normalVecs = #(#([-0.114,-0.845,0.522],[0.936,0.064,0.347],[0.815,0.239,0.529],[-0.236,-0.67,0.704]),#([-0.815,-0.239,-0.529],[0.236,0.67,-0.704],[0.936,0.064,0.347],[-0.114,-0.845,0.522]),#([-0.936,-0.064,-0.347],[-0.815,-0.239,-0.529],[-0.114,-0.845,0.522],[-0.236,-0.67,0.704]))
		nFacesCompared = faceIndices.count
		for f = 1 to nFacesCompared do
		(
			nVerticesFace = ($.modifiers[1]).GetFaceDegree faceIndices[f]
			nVerticesFaceExpected = (normalVecs[f]).count
			assert_equal nVerticesFaceExpected nVerticesFace message: "Error in face degree - Edit Poly Normal Test 3"
			for v = 1 to nVerticesFace do
			(
				indexNormal = ($.modifiers[1]).GetNormalID faceIndices[f] v
				assert_point3_equal normalVecs[f][v] (($.modifiers[1]).GetNormal indexNormal) tolerance:0.001 message: "Error in normal vector - Edit Poly Normal Test 3"
			)
		)
	),

	function EditPolyAnimatedVertexTest =
	(
		-- Test vertex animation

		resetMaxFile #noprompt
		filePath = getFileNamePath (getSourceFileName())
		fileName = filePath + "/TestData/PolyAnim.max"
		res = loadMaxFile fileName quiet:true
		assert_true res message: "Error loading file - Edit Poly vertex animation tests"

		-- Test Editable Poly animation
		format "Editable Poly vertex animation tests\n"

		select $Box002
		max modify mode
		slidertime = 0f
		Assert_point3_equal [-23.1184,-38.0403,0] ($.getvertex 1) tolerance:0.0001 message: "Error Editable Poly vert 1 frame 0"
		Assert_point3_equal [23.1184,-38.0403,0] ($.getvertex 2) tolerance:0.0001 message: "Error Editable Poly vert 2 frame 0"
		Assert_point3_equal [-23.1184,-38.0403,9.93004] ($.getvertex 5) tolerance:0.0001 message: "Error Editable Poly vert 5 frame 0"
		Assert_point3_equal [23.1184,-38.0403,9.93004] ($.getvertex 6) tolerance:0.0001 message: "Error Editable Poly vert 6 frame 0"

		slidertime = 10f
		Assert_point3_equal [-23.1184,-38.0403,2.18657] ($.getvertex 1) tolerance:0.0001 message: "Error Editable Poly vert 1 frame 10"
		Assert_point3_equal [23.1184,-38.0403,2.18657] ($.getvertex 2) tolerance:0.0001 message: "Error Editable Poly vert 2 frame 10"
		Assert_point3_equal [-23.1184,-38.0403,12.1166] ($.getvertex 5) tolerance:0.0001 message: "Error Editable Poly vert 5 frame 10"
		Assert_point3_equal [23.1184,-38.0403,12.1166] ($.getvertex 6) tolerance:0.0001 message: "Error Editable Poly vert 6 frame 10"

		slidertime = 30f
		Assert_point3_equal [-23.1184,-38.0403,13.481] ($.getvertex 1) tolerance:0.0001 message: "Error Editable Poly vert 1 frame 30"
		Assert_point3_equal [23.1184,-38.0403,13.481] ($.getvertex 2) tolerance:0.0001 message: "Error Editable Poly vert 2 frame 30"
		Assert_point3_equal [-23.1184,-38.0403,23.411] ($.getvertex 5) tolerance:0.0001 message: "Error Editable Poly vert 5 frame 30"
		Assert_point3_equal [23.1184,-38.0403,23.411] ($.getvertex 6) tolerance:0.0001 message: "Error Editable Poly vert 6 frame 30"

		-- Test Edit Poly animation
		format "Edit Poly vertex animation tests\n"

		select $Box001
		slidertime = 0f
		undo on
		(
			macros.run "Modifier Stack" "Convert_to_Poly"
			Assert_point3_equal [-16.7182,-43.3911,0] ($.getvertex 1) tolerance:0.0001 message: "Error Edit Poly vert 1 frame 0"
			Assert_point3_equal [16.7182,-43.3911,0] ($.getvertex 2) tolerance:0.0001 message: "Error Edit Poly vert 2 frame 0"
			Assert_point3_equal [-16.7182,-43.3911,10.6821] ($.getvertex 5) tolerance:0.0001 message: "Error Edit Poly vert 5 frame 0"
			Assert_point3_equal [16.7182,-43.3911,10.6821] ($.getvertex 6) tolerance:0.0001 message: "Error Edit Poly vert 6 frame 0"
		)
		max undo
		slidertime = 10f
		undo on
		(
			macros.run "Modifier Stack" "Convert_to_Poly"
			Assert_point3_equal [-16.7182,-43.3911,1.84439] ($.getvertex 1) tolerance:0.0001 message: "Error Edit Poly vert 1 frame 10"
			Assert_point3_equal [16.7182,-43.3911,1.84439] ($.getvertex 2) tolerance:0.0001 message: "Error Edit Poly vert 2 frame 10"
			Assert_point3_equal [-16.7182,-43.3911,12.5265] ($.getvertex 5) tolerance:0.0001 message: "Error Edit Poly vert 5 frame 10"
			Assert_point3_equal [16.7182,-43.3911,12.5265] ($.getvertex 6) tolerance:0.0001 message: "Error Edit Poly vert 6 frame 10"
		)
		max undo
		slidertime = 30f
		undo on
		(
			macros.run "Modifier Stack" "Convert_to_Poly"
			Assert_point3_equal [-16.7182,-43.3911,11.3713] ($.getvertex 1) tolerance:0.0001 message: "Error Edit Poly vert 1 frame 30"
			Assert_point3_equal [16.7182,-43.3911,11.3713] ($.getvertex 2) tolerance:0.0001 message: "Error Edit Poly vert 2 frame 30"
			Assert_point3_equal [-16.7182,-43.3911,22.0534] ($.getvertex 5) tolerance:0.0001 message: "Error Edit Poly vert 5 frame 30"
			Assert_point3_equal [16.7182,-43.3911,22.0534] ($.getvertex 6) tolerance:0.0001 message: "Error Edit Poly vert 6 frame 30"
		)
		max undo
	),

	function EditPolyPostprocessedExtrudeAndInsetTest =
	(
		testPostfixes = #("_2022","_2023","_2024")
		caseNames = #("a","b","c")

		-- Test "smart" extrude and inset operations with Edit Poly
		format "Edit Poly postprocessed extrude and inset tests\n"

		-- Since the postprocessed extrude and inset are inherently interactive, shift/drag operations, we cannot script a case
		-- which starts from scratch, however, by loading a test model which was built from a number of such operations, we can
		-- cause the associated PolyOperations to be performed when the Edit Poly modifiers' internal stacks are reevaluated,
		-- effectively testing the implementation
		resetMaxFile #noprompt
		local filePath = getFileNamePath (getSourceFileName())
		local fileName = filePath + "/TestData/EditPolyPostprocessedExtrudeAndInsetTest.max"
		isLoaded = loadMaxFile fileName quiet:true
		Assert_true isLoaded message: "Error loading file - Edit Poly Postprocessed Extrude and Inset Test 1"
		if (isLoaded) do
		(
			-- CompareWithBenchmark "EditPolyPostprocessedExtrudeAndInsetTest_Benchmark.max" "1" 0.0 262 228 false
		)

		-- Test a model containing an Edit Poly whose overlapping faces have edges that are nearly, but not precisely,
		-- overlapping, performing the test for both old and new version of the Smart Extrude operation; previously, the
		-- overlapping face cleanup was missed in this case due to precision issues, but now, when the Edit Poly modifier's
		-- internal stack is reevaluated, it should be treated properly
		nCases = testPostfixes.count
		for c = 1 to nCases do
		(
			resetMaxFile #noprompt
			fileName = filePath + "/TestData/EditPolyPostprocessedExtrudeNearlyParallelTest" + testPostfixes[c] + ".max"
			isLoaded = loadMaxFile fileName quiet:true
			local testName = "5" + caseNames[c]
			Assert_true isLoaded message: ("Error loading file - Edit Poly Postprocessed Extrude and Inset Test " + testName)
			if (isLoaded) do
			(
				CompareWithBenchmark ("EditPolyPostprocessedExtrudeNearlyParallelTest_Benchmark" + testPostfixes[c] + ".max") testName 0.0 30 24 false
			)
		)

		-- Test a model containing various "cut-through" inward extrusions, and "intersecting" outward extrusions; the
		-- model tests a wide range of possible scenarios, including an extrusion that cuts through a large face
		-- without intersecting any edge, an extrusion that cuts precisely through an edge of the mesh, extrusions
		-- that combine cut-through with overlap, extrusions with arbitrary direction that cut through many faces,
		-- simultaneous cut-through by multiple clusters and multi-face clusters, and intersection-type extrusions
		resetMaxFile #noprompt
		fileName = filePath + "/TestData/EditPolyPostprocessedCutThroughAndIntersectionTest.max"
		isLoaded = loadMaxFile fileName quiet:true
		Assert_true isLoaded message: "Error loading file - Edit Poly Postprocessed Extrude and Inset Test 6"
		if (isLoaded) do
		(
			CompareWithBenchmark "EditPolyPostprocessedCutThroughAndIntersectionTest_Benchmark.max" "6" 0.0 309 276 false
		)
		
		-- Test a model containing some distorted, highly non-planar faces, to ensure that the cut-through treatment
		-- does not erroneously detect them as being cut
		resetMaxFile #noprompt
		fileName = filePath + "/TestData/EditPolyPostprocessedExtrudeBadGeometryTest.max"
		isLoaded = loadMaxFile fileName quiet:true
		Assert_true isLoaded message: "Error loading file - Edit Poly Postprocessed Extrude and Inset Test 7"
		if (isLoaded) do
		(
			vertexList = #(2276,2277,2278,2279)
			faceList = #(22,2452,2453,2454,2455)
			CompareSpecifiedWithBenchmark "EditPolyPostprocessedExtrudeBadGeometryTest_Benchmark.max" "7" 0.0 "Box001" false &vertexList &faceList 1 false false
		)

		-- Test intersection of a multiple face cluster, where the first face for which the intersection is computed
		-- does not intersect any edges of the target face, performing the test for both old and new version of the Smart
		-- Extrude operation; because of the way that we decompose the resulting multiply-connected domain, intersection for
		-- adjacent faces in the cluster hits a degeneracy that was not properly handled, resulting in unstitched edges
		nCases = testPostfixes.count
		vertexLists = #(#(252,253,255,256),#(252,253,254,256,259,260,262,264,265,267,268,270,272,273,275,276),#(252,253,254,256,259,260,262,264,265,267,268,270,272,273,275,276))
		faceLists = #(#(247,248),#(240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255),#(240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255))
		for c = 1 to nCases do
		(
			resetMaxFile #noprompt
			fileName = filePath + "/TestData/EditPolyPostprocessedExtrudeMultipleFaceIntersectionTest" + testPostfixes[c] + ".max"
			isLoaded = loadMaxFile fileName quiet:true
			local testName = "8" + caseNames[c]
			Assert_true isLoaded message: ("Error loading file - Edit Poly Postprocessed Extrude and Inset Test " + testName)
			if (isLoaded) do
			(
				CompareSpecifiedWithBenchmark ("EditPolyPostprocessedExtrudeMultipleFaceIntersectionTest_Benchmark" + testPostfixes[c] + ".max") testName 0.0 "Plane001" false &(vertexLists[c]) &(faceLists[c]) 1 false false
			)
		)

		-- Test a model containing various "cut-through" inward extrusions, and "intersecting" outward extrusions; the
		-- model tests some geometric edge cases, such as cut-through of a face which is initially an edge neighbor of the
		-- extruded face, which were not addressed in the cases above
		resetMaxFile #noprompt
		fileName = filePath + "/TestData/EditPolyPostprocessedCutThroughAndIntersectionTest2.max"
		isLoaded = loadMaxFile fileName quiet:true
		Assert_true isLoaded message: "Error loading file - Edit Poly Postprocessed Extrude and Inset Test 10"
		if (isLoaded) do
		(
			CompareWithBenchmark "EditPolyPostprocessedCutThroughAndIntersectionTest2_Benchmark.max" "10" 0.0 522 438 false
		)

		-- Test an "intersecting" outward extrusion in a model with explicit normals defined on all vertices prior to the
		-- extrude operation, checking to ensure that replacement face normals are properly computed, and performing the test
		-- for both old and new version of the Smart Extrude operation
		nCases = testPostfixes.count
		for c = 1 to nCases do
		(
			resetMaxFile #noprompt
			fileName = filePath + "/TestData/EditPolyPostprocessedExtrudeExplicitNormalsTest" + testPostfixes[c] + ".max"
			isLoaded = loadMaxFile fileName quiet:true
			local testName = "11" + caseNames[c]
			Assert_true isLoaded message: ("Error loading file - Edit Poly Postprocessed Extrude and Inset Test " + testName)
			if (isLoaded) do
			(
				vertexList = #()
				faceList = #(98,99,100,101,102,103,104,105,106,107,108,109)
				CompareSpecifiedWithBenchmark ("EditPolyPostprocessedExtrudeExplicitNormalsTest_Benchmark" + testPostfixes[c] + ".max") testName 0.0 "Box001" false &vertexList &faceList 1 true true
			)
		)

		-- Test cut-though of a single target face by a multiple-face cluster, in a case where previously the cut "missed" a
		-- portion of the target face
		resetMaxFile #noprompt
		fileName = filePath + "/TestData/EditPolyPostprocessedCutThroughMultipleFaceClusterTest.max"
		isLoaded = loadMaxFile fileName quiet:true
		Assert_true isLoaded message: "Error loading file - Edit Poly Postprocessed Extrude and Inset Test 12"
		if (isLoaded) do
		(
			CompareWithBenchmark "EditPolyPostprocessedCutThroughMultipleFaceClusterTest_Benchmark.max" "12" 0.0 20 18 false
		)

		-- Test the case of multiple separate clusters being simultaneously extruded outwards and intersecting with the same
		-- non-extruded target face
		resetMaxFile #noprompt
		fileName = filePath + "/TestData/EditPolyPostprocessedExtrudeMultipleClusterIntersectionTest.max"
		isLoaded = loadMaxFile fileName quiet:true
		Assert_true isLoaded message: "Error loading file - Edit Poly Postprocessed Extrude and Inset Test 13"
		if (isLoaded) do
		(
			CompareWithBenchmark "EditPolyPostprocessedExtrudeMultipleClusterIntersectionTest_Benchmark.max" "13" 0.0 40 28 false
		)

		-- Test an outward extrusion with intersection, in a case that was initially failing due to the partitioning approach
		resetMaxFile #noprompt
		fileName = filePath + "/TestData/EditPolyPostprocessedCutThroughAndIntersectionTest3.max"
		isLoaded = loadMaxFile fileName quiet:true
		Assert_true isLoaded message: "Error loading file - Edit Poly Postprocessed Extrude and Inset Test 14"
		if (isLoaded) do
		(
			vertexList = #()
			faceList = #(40,41,42,43,44,45,69,70,71,72,73,74,97,98,99,100,124,125,126,127,153,154,155,156,182,183,184,263,264,294,295,296,297,298,299,300,301,326,327,328,329,330,331,332,333,356,357,358,359,360,361,362,363,364,365,388,389,390,391,392,393,394,395,396,397,398,421,422,423,424,425,426,427,428,429,430,431,456,457,458,459,460,461,462,535,536,537,538,539,540,541,542,543,544,545,546,547,548,549,550,551,552,553,554,555,556,557,558,559,560,561,562,563,564,565,566,567,568,569,570,571,724,725,726,727,728,729,730,731,732,733,734,735,736,737,738,739,740,741,742,743,744,745,746,747,748,749,750,751,752,753,754,755,756,757,758,759,760,761,762,763,764,765,766,767,768,769,770,771,772)
			CompareSpecifiedWithBenchmark "EditPolyPostprocessedCutThroughAndIntersectionTest3_Benchmark.max" "14" 0.0 "Sphere001" false &vertexList &faceList 1 true true
		)

		-- Test an outward extrusion with intersection, in a case which partitions into a large number of subdomains for both
		-- the mesh and replacement face bounding boxes
		resetMaxFile #noprompt
		fileName = filePath + "/TestData/EditPolyPostprocessedCutThroughAndIntersectionTest4.max"
		isLoaded = loadMaxFile fileName quiet:true
		Assert_true isLoaded message: "Error loading file - Edit Poly Postprocessed Extrude and Inset Test 15"
		if (isLoaded) do
		(
			CompareWithBenchmark "EditPolyPostprocessedCutThroughAndIntersectionTest4_Benchmark.max" "15" 0.0 1500 1000 false
		)

		-- Test a cut-through extrusion on a model with explicit normals, which previously resulted in a hang
		resetMaxFile #noprompt
		fileName = filePath + "/TestData/EditPolyPostprocessedCutThroughAndIntersectionTest5.max"
		isLoaded = loadMaxFile fileName quiet:true
		Assert_true isLoaded message: "Error loading file - Edit Poly Postprocessed Extrude and Inset Test 16"
		if (isLoaded) do
		(
			CompareWithBenchmark "EditPolyPostprocessedCutThroughAndIntersectionTest5_Benchmark.max" "16" 0.0 300 300 false
		)

		-- Test an outward extrusion with intersection on a model with explicit normals; previously, the model crashed when the
		-- Edit Poly modifier was enabled because the parent mesh for the specified normals container was not defined
		resetMaxFile #noprompt
		fileName = filePath + "/TestData/EditPolyPostprocessedCutThroughAndIntersectionTest6.max"
		isLoaded = loadMaxFile fileName quiet:true
		Assert_true isLoaded message: "Error loading file - Edit Poly Postprocessed Extrude and Inset Test 17"
		if (isLoaded) do
		(
			select $Box001
			max modify mode
			($.modifiers[1]).enabled = true
			CompareWithBenchmark "EditPolyPostprocessedCutThroughAndIntersectionTest6_Benchmark.max" "17" 0.0 500 500 false
		)

		-- Test a wide array of operations, excising both the partial cut-through and intersection implementation, as well as
		-- previously-supported cut-through and overlap operations, with both Max 2023 and 2024 files, so that the extrusion
		-- postprocessing employs both versions IMN_FACE_EXTRUDE_VER_2022 and IMN_FACE_EXTRUDE_VER_2023 of the calculation,
		-- respectively
		nVerticesCompared = #(542,553)
		nFacesCompared = #(462,459)
		testNames = #("a","b")
		nCases = testNames.count
		for c = 1 to nCases do
		(
			resetMaxFile #noprompt
			fileName = filePath + "/TestData/EditPolyPartialAndMultipleCutThroughTest1" + testPostfixes[c + 1] + ".max"
			isLoaded = loadMaxFile fileName quiet:true
			Assert_true isLoaded message: ("Error loading file - Edit Poly Postprocessed Extrude and Inset Test 18" + testNames[c])
			if (isLoaded) do
			(	
				CompareWithBenchmark ("EditPolyPartialAndMultipleCutThroughTest1_Benchmark" + testPostfixes[c + 1] + ".max") ("18" + testNames[c]) 0.0 nVerticesCompared[c] nFacesCompared[c] false
			)
		)

		-- Test an outward extrusion which passes completely through both sides of another mesh element, with an edge/edge
		-- intersections; previously, the extruded surface was truncated improperly in this case
		resetMaxFile #noprompt
		fileName = filePath + "/TestData/EditPolyPartialAndMultipleCutThroughTest2.max"
		isLoaded = loadMaxFile fileName quiet:true
		Assert_true isLoaded message: "Error loading file - Edit Poly Postprocessed Extrude and Inset Test 20"
		if (isLoaded) do
		(
			vertexList = #()
			faceList = #(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30)
			CompareSpecifiedWithBenchmark "EditPolyPartialAndMultipleCutThroughTest2_Benchmark.max" "20" 0.0 "Box001" false &vertexList &faceList 1 true true
		)

		-- Test an outward extrusion for which intersection previously was not properly detected
		resetMaxFile #noprompt
		fileName = filePath + "/TestData/EditPolyPartialAndMultipleCutThroughTest3.max"
		isLoaded = loadMaxFile fileName quiet:true
		Assert_true isLoaded message: "Error loading file - Edit Poly Postprocessed Extrude and Inset Test 21"
		if (isLoaded) do
		(
			vertexList = #()
			faceList = #(31,32,34,35,36,37,39,40,41,42,44,45,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223)
			CompareSpecifiedWithBenchmark "EditPolyPartialAndMultipleCutThroughTest3_Benchmark.max" "21" 0.0 "Box001" false &vertexList &faceList 1 true true
		)

		-- Test multiple cut-through by an inward extrusion of an isolated multi-face cluster
		nVerticesCompared = #(80,82)
		nFacesCompared = #(74,72)
		testNames = #("a","b")
		nCases = testNames.count
		for c = 1 to nCases do
		(
			resetMaxFile #noprompt
			fileName = filePath + "/TestData/EditPolyPartialAndMultipleCutThroughTest4" + testPostfixes[c + 1] + ".max"
			isLoaded = loadMaxFile fileName quiet:true
			Assert_true isLoaded message: ("Error loading file - Edit Poly Postprocessed Extrude and Inset Test 22" + testNames[c])
			if (isLoaded) do
			(
				CompareWithBenchmark ("EditPolyPartialAndMultipleCutThroughTest4_Benchmark" + testPostfixes[c + 1] + ".max") ("22" + testNames[c]) 0.0 nVerticesCompared[c] nFacesCompared[c] false
			)
		)

		-- Test a case wherein the same non-extruded face is cut through by multiple clusters in the same operation
		resetMaxFile #noprompt
		fileName = filePath + "/TestData/EditPolyPartialAndMultipleCutThroughTest5.max"
		isLoaded = loadMaxFile fileName quiet:true
		Assert_true isLoaded message: "Error loading file - Edit Poly Postprocessed Extrude and Inset Test 23"
		if (isLoaded) do
		(
			CompareWithBenchmark "EditPolyPartialAndMultipleCutThroughTest5_Benchmark.max" "23" 0.0 78 71 false
		)

		-- Test an cut-through of a complex cluster formed via a shape merge operation, which previously failed due to a
		-- difference of several orders of magnitude between cut face and cutting face areas
		resetMaxFile #noprompt
		fileName = filePath + "/TestData/EditPolyPartialAndMultipleCutThroughTest6.max"
		isLoaded = loadMaxFile fileName quiet:true
		Assert_true isLoaded message: "Error loading file - Edit Poly Postprocessed Extrude and Inset Test 24"
		if (isLoaded) do
		(
			CompareWithBenchmark "EditPolyPartialAndMultipleCutThroughTest6_Benchmark.max" "24" 0.0 110 94 false
		)

		-- Test partial intersection of a more complex geometry by a multi-face cluster
		resetMaxFile #noprompt
		fileName = filePath + "/TestData/EditPolyPartialAndMultipleCutThroughTest7.max"
		isLoaded = loadMaxFile fileName quiet:true
		Assert_true isLoaded message: "Error loading file - Edit Poly Postprocessed Extrude and Inset Test 25"
		if (isLoaded) do
		(
			CompareWithBenchmark "EditPolyPartialAndMultipleCutThroughTest7_Benchmark.max" "25" 0.0 117 74 false
		)

		-- Test multiple cut-through with a more complex geometry
		testNames = #("a","b")
		nCases = testNames.count
		for c = 1 to nCases do
		(
			resetMaxFile #noprompt
			fileName = filePath + "/TestData/EditPolyPartialAndMultipleCutThroughTest8" + testPostfixes[c + 1] + ".max"
			isLoaded = loadMaxFile fileName quiet:true
			Assert_true isLoaded message: ("Error loading file - Edit Poly Postprocessed Extrude and Inset Test 26" + testNames[c])
			if (isLoaded) do
			(
				CompareWithBenchmark ("EditPolyPartialAndMultipleCutThroughTest8_Benchmark" + testPostfixes[c + 1] + ".max") ("26" + testNames[c]) 0.0 74 58 false
			)
		)

		-- Test partial cut-through of a multi-face cluster, in a case where the extruded surface was previously not being
		-- properly truncated along the curve of intersection
		resetMaxFile #noprompt
		fileName = filePath + "/TestData/EditPolyPartialAndMultipleCutThroughTest9.max"
		isLoaded = loadMaxFile fileName quiet:true
		Assert_true isLoaded message: "Error loading file - Edit Poly Postprocessed Extrude and Inset Test 27"
		if (isLoaded) do
		(
			vertexList = #()
			faceList = #(119,120,121,122,150,151,152,180,181,182,183,184,511,512,521,522,523,524,525,534,537,538,539,540,658,659,660,661,662,663,664,665,666,667,668,669,670,671,672,673,674,675,676,677,678,679,680,681)
			CompareSpecifiedWithBenchmark "EditPolyPartialAndMultipleCutThroughTest9_Benchmark.max" "27" 0.0 "Sphere002" false &vertexList &faceList 1 true true
		)

		-- Test inward extrusion, with overlap, of a face generated by a partial cut-through operation, in two cases where a
		-- hang was previously observed
		testNames = #("a","b")
		nCases = testNames.count
		for c = 1 to nCases do
		(
			resetMaxFile #noprompt
			fileName = filePath + "/TestData/EditPolyPartialAndMultipleCutThroughTest10" + testPostfixes[c + 1] + ".max"
			isLoaded = loadMaxFile fileName quiet:true
			Assert_true isLoaded message: ("Error loading file - Edit Poly Postprocessed Extrude and Inset Test 28" + testNames[c])
			if (isLoaded) do
			(
				vertexList = #()
				faceList = #(91,93,94,95,96,97,98,100,101,103,107,108,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125)
				CompareSpecifiedWithBenchmark ("EditPolyPartialAndMultipleCutThroughTest10_Benchmark" + testPostfixes[c + 1] + ".max") ("28" + testNames[c]) 0.0 "Box003" false &vertexList &faceList 1 true true
			)
		)

		for c = 1 to nCases do
		(
			resetMaxFile #noprompt
			fileName = filePath + "/TestData/EditPolyPartialAndMultipleCutThroughTest11" + testPostfixes[c + 1] + ".max"
			isLoaded = loadMaxFile fileName quiet:true
			Assert_true isLoaded message: ("Error loading file - Edit Poly Postprocessed Extrude and Inset Test 29" + testNames[c])
			if (isLoaded) do
			(
				vertexList = #()
				faceList = #(82,83,84,87,89,127,130,131,133,150,155,156,157,158,159,160,161,162,163,164)
				CompareSpecifiedWithBenchmark ("EditPolyPartialAndMultipleCutThroughTest11_Benchmark" + testPostfixes[c + 1] + ".max") ("29" + testNames[c]) 0.0 "Box003" false &vertexList &faceList 1 true true
			)
		)

		-- Test an outward extrusion, with multiple intersections of a target surface containing significantly non-planar
		-- faces; previously, this exhibited failures in stitching and formation of the curve of intersection
		testNames = #("a","b")
		nCases = testNames.count
		for c = 1 to nCases do
		(
			resetMaxFile #noprompt
			fileName = filePath + "/TestData/EditPolyPartialAndMultipleCutThroughTest12" + testPostfixes[c + 1] + ".max"
			isLoaded = loadMaxFile fileName quiet:true
			Assert_true isLoaded message: ("Error loading file - Edit Poly Postprocessed Extrude and Inset Test 30" + testNames[c])
			if (isLoaded) do
			(
				vertexList = #()
				faceList = #(151,152,192,193,211,217,218,224,356,565,678,682,736,737,738,739,740,746,747,752,753,754,755,758,775,778,779,815,816,817,821,824,825,826,845,846,847,848,899,900,901,902,903,904,905,906,917,920,921,922,923,924,925,926)
				CompareSpecifiedWithBenchmark ("EditPolyPartialAndMultipleCutThroughTest12_Benchmark" + testPostfixes[c + 1] + ".max") ("30" + testNames[c]) 0.0 "Top" false &vertexList &faceList 1 true true
			)
		)

		-- Test the non-contiguous overlap treatment with two models, in each case, we have a multiple cut-through wherein one
		-- or more side faces of the extrusion overlap non-extruded faces which are not connected to the side face via a
		-- sequence of overlapped edge neighbor faces, so that the overlap was not detected by the original, contiguous overlap
		-- treatment
		nVerticesCompared = #(116,178)
		nFacesCompared = #(108,163)
		testNames = #("a","b")
		nonContiguousOverlapModelPostfixes = #("13","14")
		nCases = nonContiguousOverlapModelPostfixes.count
		for c = 1 to nCases do
		(
			resetMaxFile #noprompt
			fileName = filePath + "/TestData/EditPolyPartialAndMultipleCutThroughTest" + nonContiguousOverlapModelPostfixes[c] + ".max"
			isLoaded = loadMaxFile fileName quiet:true
			Assert_true isLoaded message: ("Error loading file - Edit Poly Postprocessed Extrude and Inset Test 31" + testNames[c])
			if (isLoaded) do
			(
				CompareWithBenchmark ("EditPolyPartialAndMultipleCutThroughTest" + nonContiguousOverlapModelPostfixes[c] + "_Benchmark.max") ("31" + testNames[c]) 0.0 nVerticesCompared[c] nFacesCompared[c] false
			)
		)

		-- Test an inward extrusion on a pair of models with explicit normals; previously some overlapped or truncated faces
		-- were not properly removed
		nVerticesCompared = #(56,64)
		nFacesCompared = #(48,58)
		testNames = #("a","b")
		nCases = testNames.count
		for c = 1 to nCases do
		(
			resetMaxFile #noprompt
			fileName = filePath + "/TestData/EditPolyPostprocessedExtrudeExplicitNormalsCutThroughTest" + (c as string) + ".max"
			isLoaded = loadMaxFile fileName quiet:true
			Assert_true isLoaded message: ("Error loading file - Edit Poly Postprocessed Extrude and Inset Test 32" + testNames[c])
			if (isLoaded) do
			(
				CompareWithBenchmark ("EditPolyPostprocessedExtrudeExplicitNormalsCutThroughTest" + (c as string) + "_Benchmark.max") ("32" + testNames[c]) 0.0 nVerticesCompared[c] nFacesCompared[c] false
			)
		)

		-- Test several cases where the implementation to split non-planar input mesh faces along non-flat diagonals is
		-- important; this included models with highly non-planar faces, models where imprecision due to non-planarity
		-- previously resulted in spurious cut-through detection, and cases with scale-mode insetting of non-planar faces
		nVerticesCompared = #(34,156,64,64,22,104)
		nFacesCompared = #(32,151,54,62,20,102)
		testNames = #(#("a","b","c","d","e","f"),#("g","h","i","j","k","l"))
		nVersions = 2
		for ver = 1 to nVersions do
		(
			nCasesVersion = (testNames[ver]).count
			for c = 1 to nCasesVersion do
			(
				resetMaxFile #noprompt
				fileName = filePath + "/TestData/EditPolyPostprocessedExtrudeNonPlanarTest" + (c as string) + testPostfixes[ver + 1] + ".max"
				isLoaded = loadMaxFile fileName quiet:true
				Assert_true isLoaded message: ("Error loading file - Edit Poly Postprocessed Extrude and Inset Test 33" + testNames[ver][c])
				if (isLoaded) do
				(
					CompareWithBenchmark ("EditPolyPostprocessedExtrudeNonPlanarTest" + (c as string) + "_Benchmark" + testPostfixes[ver + 1] + ".max") ("33" + testNames[ver][c]) 0.0 nVerticesCompared[c] nFacesCompared[c] false
				)
			)
		)

		-- Test a case with a very small extrusion distance, relative to the scale of the geometry, where the new zero length
		-- edge welding tolerance introduced in Max 2023 previous resulted in a malformed extrusion
		testNames = #("a","b")
		nCases = testNames.count
		for c = 1 to nCases do
		(
			resetMaxFile #noprompt
			fileName = filePath + "/TestData/EditPolyPostprocessedExtrudeWeldingToleranceTest" + testPostfixes[c + 1] + ".max"
			isLoaded = loadMaxFile fileName quiet:true
			Assert_true isLoaded message: ("Error loading file - Edit Poly Postprocessed Extrude and Inset Test 34" + testNames[c])
			if (isLoaded) do
			(
				vertexList = #()
				faceList = #(5,13,15,26,29,44,47,50,89,92,95,109,110,121,122,123,124)
				CompareSpecifiedWithBenchmark ("EditPolyPostprocessedExtrudeWeldingToleranceTest_Benchmark" + testPostfixes[c + 1] + ".max") ("34" + testNames[c]) 0.0 "Box115" false &vertexList &faceList 1 true true
			)
		)

		-- Test several cases where the implementation to split faces along diagonals at concave face corners is important;
		-- each model tested below previously failed due to degeneracy is the overlap detection and replacement face generation
		-- functions in the presence of non-convex faces
		nVerticesCompared = #(72,96,156)
		nFacesCompared = #(52,68,113)
		testNames = #("a","b","c")
		nCases = testNames.count
		for c = 1 to nCases do
		(
			resetMaxFile #noprompt
			fileName = filePath + "/TestData/EditPolyPostprocessedExtrudeConcaveTest" + (c as string) + ".max"
			isLoaded = loadMaxFile fileName quiet:true
			Assert_true isLoaded message: ("Error loading file - Edit Poly Postprocessed Extrude and Inset Test 35" + testNames[c])
			if (isLoaded) do
			(
				CompareWithBenchmark ("EditPolyPostprocessedExtrudeConcaveTest" + (c as string) + "_Benchmark.max") ("35" + testNames[c]) 0.0 nVerticesCompared[c] nFacesCompared[c] false
			)
		)

		-- Test both outward and inward extrusions on a problematic, somewhat corrupted input geometry generated by the Carve
		-- Boolean engine; previously, these cases resulted in either a crash, or grossly malformed geometry
		vertexList = #()
		faceLists = #(#(100,102,105,107,110,111,112,113,114,116,525,634,639,668,687,748,756,757,758,759,760,761,762,763,764,766,767),#(1,67,99,100,101,102,104,105,106,109,110,111,112,113,114,115,181,187,195,196,303,304,306,307,308,314,316,317,318,319,327,328,329,330,331,405,408,410,412,415,422,510,517,520,533,538,548,550,551,553,554,555,556,638,650,651,663,666,681,684,689,690,697,704,705,712,722,723,730,743,744,750,752,753,754,755,756,757,758,759,760,761,762,763,764,765,766,767,768,769,770,771,772,773,774,775,776,777,778,779))
		testNames = #("a","b")
		nCases = testNames.count
		for c = 1 to nCases do
		(
			resetMaxFile #noprompt
			fileName = filePath + "/TestData/EditPolyPostprocessedExtrudeCorruptBooleanGeometryTest" + (c as string) + ".max"
			isLoaded = loadMaxFile fileName quiet:true
			Assert_true isLoaded message: ("Error loading file - Edit Poly Postprocessed Extrude and Inset Test 36" + testNames[c])
			if (isLoaded) do
			(
				CompareSpecifiedWithBenchmark ("EditPolyPostprocessedExtrudeCorruptBooleanGeometryTest" + (c as string) + "_Benchmark.max") ("36" + testNames[c]) 0.0 "pedal_starter" false &vertexList &(faceLists[c]) 1 true true
			)
		)

		-- Test extrusions on highly complex, though valid, input geometry generated by the Carve Boolean engine; previously,
		-- these cases resulted in incomplete cut-through or otherwise malformed geometry, due to edge cases in the
		-- triangulation algorithm, and in the identification of, and truncation of the extruded surface by, the cut-through
		-- curve of intersection
		vertexList = #()
		faceLists = #(#(32,33,36,37,38,39,40,54,201,202,315,446,454,460,461,462,466,467,468,469,470,471,472,473,474,475,476,477,478,479,480,481,482,483,484),#(1,18,19,22,23,27,44,49,528,541,543,549,548,557,563,565,589,596,606,607,609,612,613,614,615,616,625,633,635,637,639,676,678,680,681,685,687,703,704,705,706,707,708,709,710,711,714,715,716,717,718,719,720,721,722,723,724))
		testNames = #(#("a","b"),#("c","d"))
		nCases = testNames.count
		for c = 1 to nCases do
		(
			resetMaxFile #noprompt
			fileName = filePath + "/TestData/EditPolyPostprocessedExtrudeBooleanGeometryTest" + (c as string) + ".max"
			isLoaded = loadMaxFile fileName quiet:true
			Assert_true isLoaded message: ("Error loading file - Edit Poly Postprocessed Extrude and Inset Test 37" + testNames[c][1])
			if (isLoaded) do
			(
				-- Compare the present result against our benchmark
				CompareSpecifiedWithBenchmark ("EditPolyPostprocessedExtrudeBooleanGeometryTest" + (c as string) + "_Benchmark.max") ("37" + testNames[c][1]) 0.0 "BooleanGeometry" true &vertexList &(faceLists[c]) 1 true true

				-- Triangulate the present result, based upon the stored face diagonal lists, and compare against our
				-- triangulated benchmark, as some of the previous issues with the present models were related to the output
				-- triangulation
				select $BooleanGeometry
				max modify mode
				triangleList = #()
				TriangulateAndBuildFaceTriangleList &triangleList &(faceLists[c])
				CompareSpecifiedWithBenchmark ("EditPolyPostprocessedExtrudeBooleanGeometryTest" + (c as string) + "_Triangulated_Benchmark.max") ("37" + testNames[c][2]) 0.0 "BooleanGeometry" false &vertexList &triangleList 1 true true
			)
		)

		-- Test extrusions of faces connected to non-extruded faces by hidden edges; previously, such non-extruded faces would
		-- have been erroneously merged into the selection
		vertexList = #()
		faceLists = #(#(17,519,525,528,530,534,535,536,537,558,559,572,581,589,590,591,593,594,595,596,597,598,599,600,601,602,603),#(544,591,592,593,594,595,596))
		testNames = #("a","b")
		nCases = testNames.count
		for c = 1 to nCases do
		(
			resetMaxFile #noprompt
			fileName = filePath + "/TestData/EditPolyPostprocessedExtrudeHiddenEdgeTest" + (c as string) + ".max"
			isLoaded = loadMaxFile fileName quiet:true
			Assert_true isLoaded message: ("Error loading file - Edit Poly Postprocessed Extrude and Inset Test 38" + testNames[c])
			if (isLoaded) do
			(
				CompareSpecifiedWithBenchmark ("EditPolyPostprocessedExtrudeHiddenEdgeTest" + (c as string) + "_Benchmark.max") ("38" + testNames[c]) 0.0 "Box001" false &vertexList &(faceLists[c]) 1 true true
			)
		)

		-- Test modification of the stack underneath an Edit Poly modifier with a Smart Extrude operation defined, in a
		-- geometrically degenerate case; previously, this model encountered a crash
		resetMaxFile #noprompt
		fileName = filePath + "/TestData/EditPolyPostprocessedExtrudeStackChangeTest.max"
		isLoaded = loadMaxFile fileName quiet:true
		Assert_true isLoaded message: "Error loading file - Edit Poly Postprocessed Extrude and Inset Test 39"
		if (isLoaded) do
		(
			max modify mode
			select $Box001

			-- Check the against our benchmark results, prior to modification of the stack
			vertexList = #()
			faceList = #(11,13,14,37,66,68,110,114,116,142,144,145,156,157,162,189,208,210,258,259,286,315,317,334,342,344,363,375,376,377,379,382,384,398,421,437,439,450,484,499,500,540,573,641,642,643,644,647,670,701,718,747,748,759,760,763,2636,2721,2810,2955,3002,3065,3105,3109,3178,3194,3238,3259,3330,3331,3339,3386,4371,4372,4373,4374,4383,4384,4385,4386)
			CompareSpecifiedWithBenchmark "EditPolyPostprocessedExtrudeStackChangeTest_Unaltered_Benchmark.max" "39a" 0.0 "Box001" true &vertexList &faceList 1 true true

			-- Disable the Symmetry modifier located directly underneath the Edit Poly modifier defining the extrusion of
			-- interest in the modifier stack
			max modify mode
			select $Box001
			($.modifiers[2]).enabled = false
			
			-- Check against our benchmark results
			vertexList = #()
			faceList = #(10,44,79,88,92,106,107,115,166,174,196,197,200,224,225,226,227,267,294,296,323,332,333,340,341,343,358,359,361,404,418,422,434,436,438,448,464,536,539,551,572,573,579,606,635,680,682,683,710,712,713,716,717,732,746,777,778,781,853,854,856,870,875,886,887,900,901,915,930,935,944,945,967,1005,1008,1017,1045,1048,1081,1082)
			CompareSpecifiedWithBenchmark "EditPolyPostprocessedExtrudeStackChangeTest_Altered_Benchmark.max" "39a" 0.0 "Box001" false &vertexList &faceList 1 true true
		)
	),
	
	
	function EditPolySmartExtrudeEnvVar = (
		testObj = box()
		addmodifier testObj (Edit_Poly())
		assert_true testObj.editpoly.enableEnhancedHotkeyExtrude  message:"Default value of Smart Extrude is incorrect"
		
		scriptPath = pathConfig.appendPath (getFilenamePath (getSourceFileName()) ) "CheckSmartExtrudeEnv.mxs"
		thisFileName = getSourceFileName()
		outputDir = pathconfig.appendpath (pathConfig.appendPath (sysinfo.tempdir) "maxauto") (getFileNameFile thisFileName)
		if not (doesfileexist outputDir) then
		(
			-- if the folder doesn't already exist, make it
			makeDir outputDir all:true
		)
		else
		(
			-- if the folder does already exist, clean up files from previous run
			files = getFiles (pathconfig.appendpath outputDir "*.*")
			for f in files do
			(
				deleteFile f
			)
		)

		logs = pathConfig.appendPath outputDir "\CheckSmartExtrudeEnv.log"

		systemTools.setEnvVariable "ADSK_3DSMAX_SMART_EXTRUDE" "0"
		
		run_max scriptPath logs
		
		delete testObj

		isSmartExtrudeSet = findTextInFile logs "ADSK_3DSMAX_SMART_EXTRUDE SET : false"
		assert_true isSmartExtrudeSet message:"Environment varible ADSK_3DSMAX_SMART_EXTRUDE is not being read"
	),
	
	function EditPolyTriangulationTest =
	(
		local nCases = 2
		local nEdgesTriangle = 3
		local boxDimension = 50.0
		local cylinderRadius = 10.0
		local caseNames = #("a","b")
		local useEnhancedRetriangulation = #(false, true)

		-- Test the legacy and revised triangulation algorithms with Edit Poly
		format "Edit Poly Triangulation Tests\n"

		-- Test triangulation of a complex, very high-degree, multiply connected polygon, for which the legacy retriangulation
		-- algorithm produced inverted triangles, with both the legacy approach our revised algorithm
		local vertexList = #()
		local faceList = #()
		local comparedFaces = #()
		local filePathTemp = GetDir #temp
		local filePath = getFileNamePath(getSourceFileName())
		local fileName = filePath + "/TestData/EditPolyTriangulationTest1.max"
		local fileNameTemp = filePathTemp + "/TempFile_EditPolyTriangulationTest1.max"
		for c = 1 to nCases do
		(
			resetMaxFile #noprompt
			local testName = "1" + caseNames[c]

			-- Load the model
			local isLoaded = loadMaxFile fileName quiet:true
			assert_true isLoaded message: ("Error Edit Poly Triangulation Test " + testName + " (load)")
			if (isLoaded) do
			(
				select $Box001
				max modify mode

				-- Add an Edit Poly modifier, and compute a triangulation for the face of interest based upon the appropriate
				-- algorithm for case c
				comparedFaces = #(27)
				local faceVertices = polyop.getFaceVerts $ comparedFaces[1]
				local nTrianglesComparedFace = (faceVertices.count) - nEdgesTriangle + 1
				addModifier $ (Edit_Poly())
				subObjectLevel = 4
				($.modifiers[1]).Select #Face #{comparedFaces[1]}
				($.modifiers[1]).enableEnhancedRetriangulation = useEnhancedRetriangulation[c]
				($.modifiers[1]).ButtonOp #Retriangulate
				update $

				-- Save the model to a temporary file
				local isSaved = false
				local isNameUnavailable = isMaxFile fileNameTemp
				if (not isNameUnavailable) do
				(
					isSaved = saveMaxFile fileNameTemp quiet:true
				)

				assert_true isSaved message: ("Error Edit Poly Triangulation Test " + testName + " (save)")

				-- Compare all triangles in the triangulation of the face of interest against our benchmark result
				TriangulateAndBuildFaceTriangleList &faceList &comparedFaces
				assert_equal nTrianglesComparedFace (faceList.count) message: ("Error Editable Poly Triangulation Test " + testName + " (number of triangles)")
 				CompareSpecifiedWithBenchmark ("EditPolyTriangulationBenchmark" + testName + ".max") testName 0.0 $.name false &vertexList &faceList 1 true true

				-- If the model was properly saved above, load the temporary file, check the triangulation of the face of
				-- interest against our benchmark again to ensure that the triangulation is correctly rebuild on load, and
				-- delete the temporary file
				if (isSaved) do
				(
					resetMaxFile #noprompt
					isLoaded = loadMaxFile fileNameTemp quiet:true
					assert_true isLoaded message: ("Error Edit Poly Triangulation Test " + testName + " (load saved file)")
					if (isLoaded) do
					(
						select $Box001
						max modify mode
						TriangulateAndBuildFaceTriangleList &faceList &comparedFaces
						assert_equal nTrianglesComparedFace (faceList.count) message: ("Error Editable Poly Triangulation Test " + testName + " (number of triangles, saved file)")
						CompareSpecifiedWithBenchmark ("EditPolyTriangulationBenchmark" + testName + ".max") (testName + " (saved file)") 0.0 $.name false &vertexList &faceList 1 true true
					)

					resetMaxFile #noprompt
					deleteFile fileNameTemp
				)
			)
		)

		-- Test triangulation of a multiply-connected non-planar face with the revised algorithm
		resetMaxFile #noprompt

		-- Create a simple 2 x 2 x 2 segment box
		local boxCurr = Box lengthsegs:2 widthsegs:2 heightsegs:2 pos:[0.0,0.0,0.0] isSelected:on length:boxDimension width:boxDimension height:boxDimension mapCoords:true
		convertTo $ PolyMeshObject
		select boxCurr
		max modify mode

		-- Remove the edges between the four faces on the "top" of the box, leaving a single degree eight face, and move the
		-- mid-side vertices on the two sides of this face along the x-direction up in z, creating a non-planar "top" face
		subobjectLevel = 2
		($.EditablePoly).SetSelection #Edge #{}
		($.EditablePoly).SetSelection #Edge #{14,15,19,20}
		($.EditablePoly).Remove()
		subobjectLevel = 1
		($.EditablePoly).SetSelection #Vertex #{}
		($.EditablePoly).SetSelection #Vertex #{11,16}
		move $.selectedVerts [0,0,0.2 * boxDimension]

		-- Create a cylinder, with its axial direction along the z-axis, through the middle of the box created above, and
		-- subtract the cylinder from the box, leaving us with a single object having a non-planar, multiply-connected "top"
		-- face, and convert to Editable Poly
		local cylinderCurr = Cylinder sides:20 capsegs:1 heightsegs:5 pos:[0.0,0.0,-0.5 * boxDimension] radius:cylinderRadius height:(2.0 * boxDimension) isSelected:on
		select boxCurr
		startObjectCreation BooleanObject2
		$.AppendOperand $ cylinderCurr false
		$.SetOperationType 2 #subtraction
		$.retainNonPlanar = true
		convertTo $ PolyMeshObject
		delete cylinderCurr
		select boxCurr
		max modify mode

		-- Add an Edit Poly modifier, compute a triangulation for the "top" face using our revised algorithm, which should be
		-- employed by default in a newly-created Edit Poly modifier
		comparedFaces = #(14)
		local nTrianglesFace = ((polyop.getFaceVerts $ comparedFaces[1]).count) - nEdgesTriangle + 1
		addModifier $ (Edit_Poly())
		assert_true ($.modifiers[1]).enableEnhancedRetriangulation message: "Error Edit Poly Triangulation Test 2 (default triangulation algorithm)"
		subObjectLevel = 4
		($.modifiers[1]).Select #Face #{comparedFaces[1]}
		($.modifiers[1]).ButtonOp #Retriangulate
		update $

		-- Compare all triangles in the triangulation of the "top" face against our benchmark result
		TriangulateAndBuildFaceTriangleList &faceList &comparedFaces
		assert_equal nTrianglesFace (faceList.count) message: "Error Editable Poly Triangulation Test 2 (number of triangles)"
		CompareSpecifiedWithBenchmark "EditPolyTriangulationBenchmark2.max" "2" 0.0 $.name false &vertexList &faceList 1 true true

		-- Test triangulation when moving vertices in a manner that invalidates the triangulation of a face at the vertex; test
		-- first with a legacy file, predating the implementation of retriangulation on vertex move, to confirm that the legacy
		-- result is recovered, then with a current file, containing a vertex move operation that triggers a retriangulation
		for c = 1 to nCases do
		(
			resetMaxFile #noprompt
			local testName = "3" + caseNames[c]

			-- Load the model
			fileName = filePath + "/TestData/EditPolyTriangulationTest" + testName + ".max"
			local isLoaded = loadMaxFile fileName quiet:true
			assert_true isLoaded message: ("Error Edit Poly Triangulation Test " + testName + " (load)")
			if (isLoaded) do
			(
				select $Box001
				max modify mode

				-- Compare all triangles in the triangulation of the "top" face against our benchmark result
				comparedFaces = #(27)
				local faceVertices = polyop.getFaceVerts $ comparedFaces[1]
				local nTrianglesComparedFace = (faceVertices.count) - nEdgesTriangle + 1
				TriangulateAndBuildFaceTriangleList &faceList &comparedFaces
				assert_equal nTrianglesComparedFace (faceList.count) message: ("Error Edit Poly Triangulation Test " + testName + " (number of triangles)")
				CompareSpecifiedWithBenchmark ("EditPolyTriangulationBenchmark" + testName + ".max") testName 0.0 $.name false &vertexList &faceList 1 true true
			)
		)

		-- Test triangulation during an edge creation operation which splits a complex, very high-degree, multiply connected
		-- face
		resetMaxFile #noprompt
		fileName = filePath + "/TestData/EditPolyTriangulationTest4.max"
		isLoaded = loadMaxFile fileName quiet:true
		assert_true isLoaded message: ("Error Edit Poly Triangulation Test 4 (load)")
		if (isLoaded) do
		(
			select $Box001
			max modify mode

			-- Compare all triangles in the triangulations of the subdomains of the face of interest against our benchmark
			-- result
			convertTo $ PolyMeshObject
			local splitFaces = #(27,10459)
			local nSplitFaceTrianglesTot = 0
			local nSplitFaces = splitFaces.count
			for f = 1 to nSplitFaces do
			(
				local splitFaceVertices = polyop.getFaceVerts $ splitFaces[f]
				local nVerticesSplitFace = splitFaceVertices.count
				nSplitFaceTrianglesTot = nSplitFaceTrianglesTot + nVerticesSplitFace - nEdgesTriangle + 1
			)

			TriangulateAndBuildFaceTriangleList &faceList &splitFaces
			assert_equal nSplitFaceTrianglesTot (faceList.count) message: ("Error Edit Poly Triangulation Test 4 (number of triangles)")
			CompareSpecifiedWithBenchmark ("EditPolyTriangulationBenchmark4.max") "4" 0.0 $.name false &vertexList &faceList 1 true true
		)

		-- Test triangulation during a slice operation which splits a complex, very high-degree, multiply connected face
		resetMaxFile #noprompt
		fileName = filePath + "/TestData/EditPolyTriangulationTest1.max"
		isLoaded = loadMaxFile fileName quiet:true
		assert_true isLoaded message: ("Error Edit Poly Triangulation Test 5 (load)")
		if (isLoaded) do
		(
			select $Box001
			max modify mode

			-- Add an Edit Poly modifier, and retriangulate the face of interest, in order to ensure that we begin with a valid
			-- triangulation
			local splitFaces = #(27)
			addModifier $ (Edit_Poly())
			subobjectLevel = 4
			($.modifiers[1]).SetSelection #Face #{}
			($.modifiers[1]).Select #Face #{splitFaces[1]}
			($.modifiers[1]).ButtonOp #Retriangulate
			update $

			-- Perform a slice operation which cuts through the face of interest, splitting it into multiple subdomain faces,
			-- which will then automatically be retriangulated with the revised algorithm, and convert to an Editable Poly
			subobjectLevel = 1
			($.modifiers[1]).SetOperation #Slice
			($.modifiers[1]).RotateSlicePlane (quat 0.0 -0.462 0.0 0.887) parent:(transMatrix [116.5,99.187,0.0]) axis:(transMatrix [116.45,99.187,0.0])
			($.modifiers[1]).Commit()
			convertTo $ PolyMeshObject

			-- Build a list of subdomain faces into which the face of interest was split, based upon selection propagation
			subobjectLevel = 4
			splitFaces.count = 0
			local isSplitFace = polyop.getFaceSelection $
			local nFacesTot = $.numFaces
			for f = 1 to nFacesTot do
			(
				if (isSplitFace[f]) do
				(
					append splitFaces f
				)
			)

			assert_equal 7 (splitFaces.count) message: ("Error Edit Poly Triangulation Test 5 (number of split faces)")

			-- Compare all triangles in the triangulations of the subdomains of the face of interest against our benchmark
			-- result
			local nSplitFaceTrianglesTot = 0
			local nSplitFaces = splitFaces.count
			for f = 1 to nSplitFaces do
			(
				local splitFaceVertices = polyop.getFaceVerts $ splitFaces[f]
				local nVerticesSplitFace = splitFaceVertices.count
				nSplitFaceTrianglesTot = nSplitFaceTrianglesTot + nVerticesSplitFace - nEdgesTriangle + 1
			)

			TriangulateAndBuildFaceTriangleList &faceList &splitFaces
			assert_equal nSplitFaceTrianglesTot (faceList.count) message: ("Error Edit Poly Triangulation Test 5 (number of triangles)")
			CompareSpecifiedWithBenchmark ("EditPolyTriangulationBenchmark5.max") "5" 0.0 $.name false &vertexList &faceList 1 true true
		)

		-- Test triangulation during a cut operation which splits a complex, very high-degree, multiply connected face
		resetMaxFile #noprompt
		fileName = filePath + "/TestData/EditPolyTriangulationTest1.max"
		isLoaded = loadMaxFile fileName quiet:true
		assert_true isLoaded message: ("Error Edit Poly Triangulation Test 6 (load)")
		if (isLoaded) do
		(
			select $Box001
			max modify mode
			
			-- Add an Edit Poly modifier, and retriangulate the face of interest, in order to ensure that we begin with a valid
			-- triangulation
			local splitFaces = #(27)
			addModifier $ (Edit_Poly())
			subobjectLevel = 4
			($.modifiers[1]).SetSelection #Face #{}
			($.modifiers[1]).Select #Face #{splitFaces[1]}
			($.modifiers[1]).ButtonOp #Retriangulate
			update $

			-- Perform a cut operation on the face of interest, splitting it into two subdomain faces, which will then
			-- automatically be retriangulated with the revised algorithm, and convert to an Editable Poly
			subobjectLevel = 1
			($.modifiers[1]).SetOperation #Cut
			($.modifiers[1]).StartCut #Edge 614 [21.766,130.697,116.859] [-0.16,-0.552,0.819]
			($.modifiers[1]).SetCutEnd [1.687,34.459,116.859]
			convertTo $ PolyMeshObject

			-- Build a list of subdomain faces into which the face of interest was split, based upon selection propagation
			splitFaces.count = 0
			local isSplitFace = polyop.getFaceSelection $
			local nFacesTot = $.numFaces
			for f = 1 to nFacesTot do
			(
				if (isSplitFace[f]) do
				(
					append splitFaces f
				)
			)

			assert_equal 2 (splitFaces.count) message: ("Error Edit Poly Triangulation Test 6 (number of split faces)")

			-- Compare all triangles in the triangulations of the subdomains of the face of interest against our benchmark
			-- result
			local nSplitFaceTrianglesTot = 0
			local nSplitFaces = splitFaces.count
			for f = 1 to nSplitFaces do
			(
				local splitFaceVertices = polyop.getFaceVerts $ splitFaces[f]
				local nVerticesSplitFace = splitFaceVertices.count
				nSplitFaceTrianglesTot = nSplitFaceTrianglesTot + nVerticesSplitFace - nEdgesTriangle + 1
			)

			TriangulateAndBuildFaceTriangleList &faceList &splitFaces
			assert_equal nSplitFaceTrianglesTot (faceList.count) message: ("Error Edit Poly Triangulation Test 6 (number of triangles)")
			CompareSpecifiedWithBenchmark ("EditPolyTriangulationBenchmark6.max") "6" 0.0 $.name false &vertexList &faceList 1 true true
		)

		-- Test triangulation during an edge creation operation which splits a complex, very high-degree, multiply connected
		-- face, loaded from a legacy file
		resetMaxFile #noprompt
		fileName = filePath + "/TestData/EditPolyTriangulationTest7.max"
		isLoaded = loadMaxFile fileName quiet:true
		assert_true isLoaded message: ("Error Edit Poly Triangulation Test 7 (load)")
		if (isLoaded) do
		(
			select $Box001
			max modify mode

			-- Build a list of subdomain faces into which the face of interest was split, based upon selection propagation
			local splitFaces = #()
			convertTo $ PolyMeshObject
			subobjectLevel = 4
			local isSplitFace = polyop.getFaceSelection $
			local nFacesTot = $.numFaces
			for f = 1 to nFacesTot do
			(
				if (isSplitFace[f]) do
				(
					append splitFaces f
				)
			)
			
			assert_equal 2 (splitFaces.count) message: ("Error Edit Poly Triangulation Test 7 (number of split faces)")

			-- Compare all triangles in the triangulations of the subdomains of the face of interest against our benchmark
			-- result
			local nSplitFaceTrianglesTot = 0
			local nSplitFaces = splitFaces.count
			for f = 1 to nSplitFaces do
			(
				local splitFaceVertices = polyop.getFaceVerts $ splitFaces[f]
				local nVerticesSplitFace = splitFaceVertices.count
				nSplitFaceTrianglesTot = nSplitFaceTrianglesTot + nVerticesSplitFace - nEdgesTriangle + 1
			)

			TriangulateAndBuildFaceTriangleList &faceList &splitFaces
			assert_equal nSplitFaceTrianglesTot (faceList.count) message: ("Error Edit Poly Triangulation Test 7 (number of triangles)")
			CompareSpecifiedWithBenchmark ("EditPolyTriangulationBenchmark7.max") "7" 0.0 $.name false &vertexList &faceList 1 true true
		)
	),

	Function EditPolyCapHolesTest =
	(
		local caseNames = #("a","b")
		local useEnhancedRetriangulation = #(false, true)
		
		-- Test hole capping operations with Edit Poly
		format "Edit Poly Cap Holes Tests\n"

		-- Test loading of a model containing an Edit Poly modifier, defining a Cap Holes operation, which was
		-- generated prior to the implementation of the enhanced retriangulation algorithm in Edit Poly
		resetMaxFile #noPrompt
		local filePath = getFileNamePath(getSourceFileName())
		local fileName = filePath + "/TestData/EditPolyCapHolesTest1.max"
		local isLoaded = loadMaxFile fileName quiet:true
		assert_true isLoaded message: "Error test case 2b (load)"
		if (isLoaded) do
		(
			select $Box001
			max modify mode

			-- Convert to a purely triangular mesh, employing the triangulation generated by the Cap Holes modifier
			TriangulateSelectedObject()

			-- Compare all triangles in the triangulation of the output mesh against our benchmark result
			local vertexList = #()
			local faceList = #()
			local nFaces = $.numFaces
			faceList.count = nFaces
			for f = 1 to nFaces do
			(
				faceList[f] = f
			)

			CompareSpecifiedWithBenchmark "EditPolyCapHolesBenchmark1.max" "1" 0.0 $.name false &vertexList &faceList 1 true true
		)

		-- Test capping of a hole with a grossly non-planar bounding curve via Edit Poly, employing both the legacy and
		-- enhanced retriangulation algorithms
		local nCases = caseNames.count
		for c = 1 to nCases do
		(
			resetMaxFile #noPrompt
			local caseName = "2" + caseNames[c]
			
			-- Build a single segment box
			local boxCurr = Box lengthsegs:1 widthsegs:1 heightsegs:1 pos:[0.0,0.0,0.0] isSelected:on length:50.0 width:50.0 height:50.0 mapCoords:true
			select boxCurr
			max modify mode

			-- Convert to Editable Poly, and remove a pair of edge neighbor faces defining two sides of the box
			convertTo $ PolyMeshObject
			subobjectLevel = 4
			polyop.deleteFaces $ #(2,3)

			-- Select the edges along the bounding curve of the hole created by the face deletion performed above
			subobjectLevel = 2
			$.EditablePoly.SetSelection #Edge #{}
			$.EditablePoly.SetSelection #Edge #{4,5,6,7,8,9}

			-- Apply an Edit Poly modifier, set the appropriate triangulation algorithm for case c, and cap the hole
			-- bounded by the selected edge loop
			addModifier $ (Edit_Poly())
			($.modifiers[1]).enableEnhancedRetriangulation = useEnhancedRetriangulation[c]
			subobjectLevel = 3
			($.modifiers[1]).ButtonOp #Cap

			-- Convert to a purely triangular mesh, employing the triangulation generated by the Cap Holes modifier
			TriangulateSelectedObject()

			-- Compare all triangles in the triangulation of the output mesh against our benchmark result
			local vertexList = #()
			local faceList = #()
			local nFaces = $.numFaces
			faceList.count = nFaces
			for f = 1 to nFaces do
			(
				faceList[f] = f
			)

			CompareSpecifiedWithBenchmark ("EditPolyCapHolesBenchmark" + caseName + ".max") caseName 0.0 $.name false &vertexList &faceList 1 true true
		)
	),

	Tests =
	#(
		editPoly_MAXX_10215,
		editPoly_MAXX_12834_01,
		editPoly_MAXX_12834_02,
		EditPolyNormalTest,
		EditPolyPostprocessedExtrudeAndInsetTest,
		EditPolyAnimatedVertexTest,
		EditPolySmartExtrudeEnvVar,
		EditPolyTriangulationTest,
		EditPolyCapHolesTest
	)
)

/*
foo = Obj_Prop_UnitTest()
foo.setup()
foo.editPoly_MAXX_12834_01()
foo.editPoly_MAXX_12834_02()
*/

run_test_fixture Obj_Prop_UnitTest script:(getThisScriptFilename())



