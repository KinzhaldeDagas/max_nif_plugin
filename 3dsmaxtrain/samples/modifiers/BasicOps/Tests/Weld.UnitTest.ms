--##################################################
-- Modifiers Weld Unit Test
-- Author: Mathew Kaustinen
-- Created: October 2024
--##################################################
--##################################################

struct Weld_UnitTest
(
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
		Simple Mesh Weld Test
	*/
	function Mesh_Weld_Test1 =
	(
		resetMaxFile #noprompt
		filePath = getFileNamePath (getSourceFileName())
		fileName = filePath + "/TestData/Weld_Test_Mesh.max"
		res = loadMaxFile fileName quiet:true useFileUnits:true
		assert_true res message: "Error loading file - Weld_Test_Mesh"
		
		select $Sphere001
		collapse $
		--convertTo $ TriMeshGeometry
			
		-- Check face and vert count
		local numFaces = $.numFaces
		Assert_equal numFaces 896 message: "Mesh Weld Error - Bad Face Count. Mesh_Weld_Test1"
		local numVerts = $.numVerts
		Assert_equal numVerts 450 message: "Mesh Weld Error - Bad Vert Count. Mesh_Weld_Test1"		
	),
 	

	/*
		Simple Mesh Weld Test with Stack Selection
	*/
	function Mesh_Weld_Test2 =
	(
		resetMaxFile #noprompt
		filePath = getFileNamePath (getSourceFileName())
		fileName = filePath + "/TestData/Weld_Test_Mesh_Selected.max"
		res = loadMaxFile fileName quiet:true useFileUnits:true
		assert_true res message: "Error loading file - Weld_Test_Mesh_Selected"
		
		select $Sphere001
		--convertTo $ TriMeshGeometry
		collapse $
			
		-- Check face and vert count
		local numFaces = $.numFaces
		Assert_equal numFaces 928 message: "Mesh Weld Error - Bad Face Count. Mesh_Weld_Test2"
		local numVerts = $.numVerts
		Assert_equal numVerts 466 message: "Mesh Weld Error - Bad Vert Count. Mesh_Weld_Test2"
	),
	
	/*
		Simple Poly Weld Test
	*/
	function Poly_Weld_Test1 =
	(
		resetMaxFile #noprompt
		filePath = getFileNamePath (getSourceFileName())
		fileName = filePath + "/TestData/Weld_Test_Poly.max"
		res = loadMaxFile fileName quiet:true useFileUnits:true
		assert_true res message: "Error loading file - Weld_Test_Poly"
		
		select $Sphere001
		convertTo $ PolyMeshObject
			
		-- Check face and vert count
		local numFaces = $.numFaces
		Assert_equal numFaces 454 message: "Poly Weld Error - Bad Face Count. Poly_Weld_Test1"
		local numVerts = $.numVerts
		Assert_equal numVerts 424 message: "Poly Weld Error - Bad Vert Count. Poly_Weld_Test1"		
	),
 	

	/*
		Simple Poly Weld Test with Stack Selection
	*/
	function Poly_Weld_Test2 =
	(
		resetMaxFile #noprompt
		filePath = getFileNamePath (getSourceFileName())
		fileName = filePath + "/TestData/Weld_Test_Poly_Selected.max"
		res = loadMaxFile fileName quiet:true useFileUnits:true
		assert_true res message: "Error loading file - Weld_Test_Poly_Selected"
		
		select $Sphere001
		convertTo $ PolyMeshObject
			
		-- Check face and vert count
		local numFaces = $.numFaces
		Assert_equal numFaces 483 message: "Poly Weld Error - Bad Face Count. Poly_Weld_Test2"
		local numVerts = $.numVerts
		Assert_equal numVerts 453 message: "Poly Weld Error - Bad Vert Count. Poly_Weld_Test2"
	),
	
	/*
		Check loading legacy poly welds which have DEAD_VERTS
	*/
	function Poly_Weld_Test3 =
	(
		resetMaxFile #noprompt
		filePath = getFileNamePath (getSourceFileName())
		fileName = filePath + "/TestData/Weld_Test_Legacy_Poly.max"
		res = loadMaxFile fileName quiet:true useFileUnits:true
		assert_true res message: "Error loading file - Weld_Test_Poly"
		
		select $Sphere001
		convertTo $ PolyMeshObject
			
		-- Check face and vert count
		local numFaces = $.numFaces
		Assert_equal numFaces 512 message: "Poly Weld Error - Bad Face Count. Poly_Weld_Test3"
		local numVerts = $.numVerts
		Assert_equal numVerts 482 message: "Poly Weld Error - Bad Vert Count. Poly_Weld_Test3"		
	),
	
	/*
		Simple Single Spline Weld
	*/
	function Spline_Weld_Test1 =
	(
		resetMaxFile #noprompt
		filePath = getFileNamePath (getSourceFileName())
		fileName = filePath + "/TestData/Weld_Test_Spline_Single.max"
		res = loadMaxFile fileName quiet:true useFileUnits:true
		assert_true res message: "Error loading file - Weld_Test_Spline_Single"
			
		select $Arc001
		convertToSplineShape $
			
		-- Check Knot Count
		Assert_equal (numKnots $ 1) 3 message: "Spline Weld Error - Bad Knot Count. Weld_Test_Spline_Single"
			
		-- Check first vertex position and handles
		knot_array = #([-29.208,19.0399,0], [-33.9469,14.8439,0], [-37.9454,9.93728,0])
		
		assert_point3_equal knot_array[1] (getInVec $ 1 1) tolerance:0.0001 message: "Error Spline Weld InVec 1"
		assert_point3_equal knot_array[2] (getKnotPoint $ 1 1) tolerance:0.0001 message: "Error Spline Weld Knot 1"
		assert_point3_equal knot_array[3] (getOutVec $ 1 1) tolerance:0.0001 message: "Error Spline Weld OutVec 1"		
	),
	
	/*
		Spline Weld With Selection
	*/
	function Spline_Weld_Test2 =
	(
		resetMaxFile #noprompt
		filePath = getFileNamePath (getSourceFileName())
		fileName = filePath + "/TestData/Weld_Test_Spline_Selected.max"
		res = loadMaxFile fileName quiet:true useFileUnits:true
		assert_true res message: "Error loading file - Weld_Test_Spline_Selected"
			
		select $Circle001
		convertToSplineShape $

		-- Check Spline Count
		Assert_equal (numSplines $) 2 message: "Spline Weld Error - Bad Spline Count. Weld_Test_Spline_Selected"
			
		-- Check Point Count
		Assert_equal (numKnots $ 1) 4 message: "Spline Weld Error - Bad Knot Count. Weld_Test_Spline_Selected"
		Assert_equal (numKnots $ 2) 2 message: "Spline Weld Error - Bad Knot Count. Weld_Test_Spline_Selected"
			
		-- Check segment count
		Assert_equal (numsegments $ 1) 3 message: "Spline Weld Error - Bad Segment Count. Weld_Test_Spline_Selected"
		Assert_equal (numsegments $ 2) 1 message: "Spline Weld Error - Bad Segment Count. Weld_Test_Spline_Selected"
	),
	
	/*
		Spline Weld with 2 complex closed shapes
	*/
	function Spline_Weld_Test3 =
	(
		resetMaxFile #noprompt
		filePath = getFileNamePath (getSourceFileName())
		fileName = filePath + "/TestData/Weld_Test_Spline_Letter.max"
		res = loadMaxFile fileName quiet:true useFileUnits:true
		assert_true res message: "Error loading file - Weld_Test_Spline_Letter"
			
		select $mermaid
		convertToSplineShape $
			
		-- Make sure we're closed
		assert_true (isClosed $ 1) message: "Spline Weld Error - Not Closed. Weld_Test_Spline_Letter"
		assert_true (isClosed $ 2) message: "Spline Weld Error - Not Closed. Weld_Test_Spline_Letter"
			
		-- Check Spline Count
		Assert_equal (numSplines $) 2 message: "Spline Weld Error - Bad Spline Count. Weld_Test_Spline_Letter"
			
		-- Check Point Count
		Assert_equal (numKnots $ 1) 1432 message: "Spline Weld Error - Bad Knot Count. Weld_Test_Spline_Letter"
		Assert_equal (numKnots $ 2) 527 message: "Spline Weld Error - Bad Knot Count. Weld_Test_Spline_Letter"
			
		-- Check segment count
		Assert_equal (numsegments $ 1) 1432 message: "Spline Weld Error - Bad Segment Count. Weld_Test_Spline_Letter"
		Assert_equal (numsegments $ 2) 527 message: "Spline Weld Error - Bad Segment Count. Weld_Test_Spline_Letter"
	),
	
	/*
		Spline Weld with 2 complex shapes
	*/
	function Spline_Weld_Test4 =
	(
		resetMaxFile #noprompt
		filePath = getFileNamePath (getSourceFileName())
		fileName = filePath + "/TestData/Weld_Test_Spline_Fence.max"
		res = loadMaxFile fileName quiet:true useFileUnits:true
		assert_true res message: "Error loading file - Weld_Test_Spline_Fence"
			
		select $Fence_Section_01
		convertToSplineShape $
			
		-- Make sure we're open
		assert_false (isClosed $ 1) message: "Spline Weld Error - Not Closed. Weld_Test_Spline_Fence"
		assert_false (isClosed $ 2) message: "Spline Weld Error - Not Closed. Weld_Test_Spline_Fence"
			
		-- Check Spline Count
		Assert_equal (numSplines $) 100 message: "Spline Weld Error - Bad Spline Count. Weld_Test_Spline_Fence"
			
		-- Check Point Count
		Assert_equal (numKnots $ 1) 101 message: "Spline Weld Error - Bad Knot Count. Weld_Test_Spline_Fence"
		Assert_equal (numKnots $ 2) 101 message: "Spline Weld Error - Bad Knot Count. Weld_Test_Spline_Fence"
			
		-- Check segment count
		Assert_equal (numsegments $ 1) 100 message: "Spline Weld Error - Bad Segment Count. Weld_Test_Spline_Fence"
		Assert_equal (numsegments $ 2) 100 message: "Spline Weld Error - Bad Segment Count. Weld_Test_Spline_Fence"
	),

	Tests =
	#(
		Mesh_Weld_Test1,
		Mesh_Weld_Test2,
		Poly_Weld_Test1,
		Poly_Weld_Test2,
		Poly_Weld_Test3,
		Spline_Weld_Test1,
		Spline_Weld_Test2,
		Spline_Weld_Test3,
		Spline_Weld_Test4
	)
)

if undefined ==  run_test_fixture  then 
( 
	format "-- No fixture!\n" 
	testList = Weld_UnitTest()
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
	run_test_fixture Weld_UnitTest script:(getThisScriptFilename())
) 

