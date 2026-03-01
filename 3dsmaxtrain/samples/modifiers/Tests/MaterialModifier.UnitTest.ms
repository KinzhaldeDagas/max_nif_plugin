--##################################################
-- Material Modifier Unit Test
-- Author:	Mathew Kaustinen
-- Created:	November 2, 2022
--##################################################

Struct  MaterialModUnitTests 
( 
	Function setup = 
	( 
		resetmaxFile #noPrompt

		-- load our sample file
		local filePath = getFileNamePath(getSourceFileName())
		local fileName = filePath + "/TestData/MaterialModTestBenchmark1.max"
		local isLoaded = loadMaxFile fileName quiet:true
		assert_true isLoaded message: ("Material Modifier Test Scene Load Failure")

	), 

	Function teardown = 
	( 
	),

	Function materialModifierMeshMatIDTest = 
	(
		-- Confirm that mat id on the mesh object is correct for both selected and non selected faces
		local matid1307 = getfacematid $TestMesh 1307
		Assert_equal 1 matid1307 message: "Error Mesh NonSelected MatID Failure"

		local matid1563 = getfacematid $TestMesh 1563
		Assert_equal 2 matid1563 message: "Error Mesh Selected MatID Failure"
	),

	Function materialModifierPolyMatIDTest = 
	(
		-- Confirm that mat id on the poly object is correct for both selected and non selected faces
		local matid1307 = polyop.getFaceMatID $TestPoly 1307
		Assert_equal 2 matid1307 message: "Error Poly Selected MatID Failure"

		local matid1563 = polyop.getFaceMatID $TestPoly 1563
		Assert_equal 1 matid1563 message: "Error Poly NonSelected MatID Failure"
	),

	Function materialModifierMeshNormalTest = 
	(
		-- Confirm that face normals on the mesh object is correct for both selected and non selected faces
		local norm1307 = getfacenormal $TestMesh 1307
		Assert_point3_equal [0.497466,0.69583,0.518023] norm1307 message: "Error Mesh NonSelected Normal Failure"

		local norm1563 = getfacenormal $TestMesh 1563
		Assert_point3_equal [0,-0.713015,0.701148] norm1563 message: "Error Mesh Selected Normal Failure"
	),

	Function materialModifierPolyNormalTest = 
	(
		-- Confirm that face normals on the poly object is correct for both selected and non selected faces
		local norm1307 = polyop.getFaceNormal $TestPoly 1307
		Assert_point3_equal [0.497466,0.69583,0.518023] norm1307 message: "Error Poly Selected Normal Failure"

		local norm1563 = polyop.getFaceNormal $TestPoly 1563
		Assert_point3_equal [0,-0.713015,0.701148] norm1563 message: "Error Poly NonSelected Normal Failure"
	),

	Function materialModifierSplineTest = 
	(
		-- Native spline support. Confirm that matid for segment 1 is 1 and segement 3 is 2
		converttosplineshape $TestSpline
		local matid1 = getMaterialID $TestSpline 1 1
		Assert_equal 1 matid1 message: "Error Spline Segment 1 MatID failure"
		local matid3 = getMaterialID $TestSpline 1 3
		Assert_equal 2 matid3 message: "Error Spline Segment 3 MatID failure"
	),
	 
	Tests  = #(
		materialModifierMeshMatIDTest,
		materialModifierPolyMatIDTest,
		materialModifierMeshNormalTest,
		materialModifierPolyNormalTest,
		materialModifierSplineTest
	)
)

if undefined ==  run_test_fixture  then 
( 
	format "-- No fixture!\n" 

	foo =  MaterialModUnitTests  () 
	
	foo.setup()

	for test in foo.Tests do
		(
		test()
		)

	foo.teardown()
) 
else 
( 
	format "-- Fixture found!\n" 
	run_test_fixture  MaterialModUnitTests script:(getThisScriptFilename()) 
) 

