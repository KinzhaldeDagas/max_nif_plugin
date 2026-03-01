/*
    @param TestName:      EditPoly_SceneScale.UnitTest
    @param Author:        Tom Hudson
    @param Created:       2019.07.23
    @param LastMod:         
    @param ModBy:           
    @param Description:   Edit Poly Scene Scale unit tests
    @param Status:        Online
*/

fileIn "MxsUnit/MxsUnit.ms"

struct UnitTest_EditPoly_SceneScale
(

public
	function setup =
	(
		resetmaxFile #noPrompt
	),

	function teardown =
	(
	),
	
	fn test_Load_Weld_Scale= 
	(
		format "test_Load_Weld_Scale\n"
		
		-- Grab an empty scene set to 1 Unit = 1 Inch (same as test file)
		format "1 Unit = 1 Inch Scene...\n"
		local filePath = getFileNamePath (getSourceFileName())
		local fileName = filePath + "/Tests/TestData/EmptyScene1In.max"
		res = loadMaxFile fileName useFileUnits:true quiet:true
		assert_true res message: "Error EditPoly_SceneScale Load File test case 1"

		-- Now merge in a scene with the same scale
		fileName = filePath + "/Tests/TestData/WeldTest1.max"
		res = mergeMaxFile fileName quiet:true
		assert_true res message: "Error EditPoly_SceneScale Merge File test case 1"
		
		-- Flatten the mesh and check vertex count to verify proper weld operation
		select $'Sphere001'
		Max Modify mode
		subObjectLevel = 1
		showEndResult = true
		verts = $.numVerts
		Assert_equal 1423 verts message: "Error EditPoly_SceneScale test case 1"
			
		-- Grab an empty scene set to 1 Unit = 1 Cm
		format "1 Unit = 1 Cm Scene...\n"
		filePath = getFileNamePath (getSourceFileName())
		fileName = filePath + "/Tests/TestData/EmptyScene1Cm.max"
		res = loadMaxFile fileName useFileUnits:true quiet:true
		assert_true res message: "Error EditPoly_SceneScale Load File test case 2"

		-- Now merge in a scene with different scale
		fileName = filePath + "/Tests/TestData/WeldTest1.max"
		res = mergeMaxFile fileName quiet:true
		assert_true res message: "Error EditPoly_SceneScale Merge File test case 2"
		
		-- Flatten the mesh and check vertex count to verify proper weld operation
		select $'Sphere001'
		Max Modify mode
		subObjectLevel = 1
		showEndResult = true
		verts = $.numVerts
		Assert_equal 1423 verts message: "Error EditPoly_SceneScale test case 2"
	),	
	
	Tests = 
	#(		
		test_Load_Weld_Scale  -- test weld scale
	)
)

if undefined ==  run_test_fixture  then 
( 
	format "-- No fixture!\n" 

	foo =  UnitTest_EditPoly_SceneScale  () 
	for test in foo.Tests do
		(
		foo.setup()
		test()
		foo.teardown()
		)
) 
else 
( 
	format "-- Fixture found!\n" 
	run_test_fixture  UnitTest_EditPoly_SceneScale  script:(getThisScriptFilename()) 
) 
