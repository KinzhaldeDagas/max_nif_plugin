
--##################################################
-- Modifiers Displace Unit Test
-- Author:	 Mathew Kaustinen
-- Created:	November 24, 2024
--##################################################

fileIn "MxsUnit/MxsUnit.ms"
fileIn ("CompareWithBenchmark.mxs")
fileIn ("ValidateObjectBoundsAtTime.mxs")

struct Displace_UnitTest
(
	--##################################################
	-- Unit tests
	--##################################################
public
	function setup =
	(
		resetMaxFile #noPrompt
	),

	function teardown =
	(
		
	),

	-- Builds a the longest possible list of indices indexList on the range [indexMin, indexMax] with spacing indexIncrement,
	-- so that indexList[i] = 1 + (i - 1) * indexIncrement.
	--
	-- indexList:          evenly-spaced list of indices on the range [indexMin, indexMax], with spacing indexIncrement, or
	--                     an empty array if an empty range or invalid, non-positive increment are specified; output
	-- indexMin, indexMax: bounding values of the range of indices [indexMin, indexMax] for which a list is to be generated;
	--                     input
	-- indexIncrement:     increment defining spacing of values in indexList; input
	function BuildIndexListOnRange &indexList indexMin indexMax indexIncrement =
	(
		indexList = #()
		if (indexMax >= indexMin) do
		(
			if (indexIncrement > 0) do
			(
				nValsList = 1 + (((indexMax - indexMin) / indexIncrement) as integer)
				indexList.count = nValsList
				for i = 1 to nValsList do
				(
					indexList[i] = 1 + (i - 1) * indexIncrement
				)
			)
		)
	),

	function Displace_Test testName =
	(
		format "%\n" testName
		filePath = getFileNamePath(getSourceFileName())
		fileName = filePath + "TestData/" + testName + ".max"
		isLoaded = loadMaxFile fileName quiet:true useFileUnits:true
		Assert_true isLoaded message: ("Error " + testName + " (load)")
		if (isLoaded) do
		(
			-- convert to poly so we can compare
			select $Test_Object
			convertTo $ PolyMeshObject

			-- Compare against our benchmark result
			local vertexList = #()
			local faceList = #()
			local nFaces = $.numFaces
			BuildIndexListOnRange &faceList 1 nFaces 1

			CompareWithBenchmark (testName + "_Benchmark.max") testName 0.0 $.name false &vertexList &faceList 1 false false
		)
	),
	
	function Displace_Mesh_Test =
	(
		Displace_Test ("Displace_mesh")
	),
	
	function Displace_Poly_Test =
	(
		Displace_Test ("Displace_poly")
	),
	
	function Displace_Patch_Test =
	(
		Displace_Test ("Displace_patch")
	),
	
	function Displace_Moved_Gizmo_Test =
	(
		Displace_Test ("Displace_moved_gizmo")
	),
	
	function Displace_Space_Warp_Test =
	(
		testName = "Displace_spacewarp"
		format "%\n" testName
		filePath = getFileNamePath(getSourceFileName())
		fileName = filePath + "TestData/" + testName + ".max"
		isLoaded = loadMaxFile fileName quiet:true useFileUnits:true
		Assert_true isLoaded message: ("Error " + testName + " (load)")
		if (isLoaded) do
		(
			select $Test_Object
			-- Check bounds at time 0
			ValidateObjectBoundsAtTime $ 0f #([-55.603,-45.592,-54.6904], [53.8779,59.5111,55.3227]) (testName + " Bounds error1")
			-- And again at time 50
			ValidateObjectBoundsAtTime $ 50f #([-57.6556,-43.6788,-54.5385], [52.422,60.2952,54.3316]) (testName + " Bounds error2")		
		)
	),
	
	function Displace_Animated_Mesh_Test =
	(
		testName = "Displace_simple_animated"
		format "%\n" testName
		filePath = getFileNamePath(getSourceFileName())
		fileName = filePath + "TestData/" + testName + ".max"
		isLoaded = loadMaxFile fileName quiet:true useFileUnits:true
		Assert_true isLoaded message: ("Error " + testName + " (load)")
		if (isLoaded) do
		(
			select $Test_Object
			-- Check bounds at time 0
			ValidateObjectBoundsAtTime $ 0f #([-31.2358,-50.309,-1.57339], [51.9481,25.8981,12.4461]) (testName + " Bounds error1")
			-- At time 25
			ValidateObjectBoundsAtTime $ 25f #([-31.2358,-50.309,-6.38949], [51.9481,25.8981,16.1769]) (testName + " Bounds error2")		
			-- and time 50
			ValidateObjectBoundsAtTime $ 50f #([-31.2358,-50.309,-15.1164], [51.9481,25.8981,22.7101]) (testName + " Bounds error3")		
		)
	),
	
	Tests =
	#(
		Displace_Mesh_Test,
		Displace_Poly_Test,
		Displace_Patch_Test,
		Displace_Moved_Gizmo_Test,
		Displace_Space_Warp_Test,
		Displace_Animated_Mesh_Test
	)
)

if undefined ==  run_test_fixture  then 
( 
	format "-- No fixture!\n" 
	testList = Displace_UnitTest()
	for test in testList.Tests do
	(
		testList.setup()
		test()
		testList.teardown()
	)
) 
else 
( 
	run_test_fixture Displace_UnitTest script:(getThisScriptFilename())	
)