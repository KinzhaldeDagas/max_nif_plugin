
--##################################################
-- Modifiers FFD Unit Test
-- Author:	 Larry Minton
-- Created:	May 25, 2018
-- Updated Feb 26, 2023 to compare all FDD types with expect output (M. Kaustinen)
--##################################################

fileIn "MxsUnit/MxsUnit.ms"
fileIn ("CompareWithBenchmark.mxs")

struct FFD_UnitTest
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

	function SpaceFFD_animated_cp_with_modifier_test =
	(
		--Defect MAXX-42020 Crash when assigning some modifiers to animated FFD space warp
		ffdb = SpaceFFDBox()
		animateAll  ffdb
		with animate on at time 100 for p in (getsubanimnames ffdb.baseobject[#point_controller_container]) do setProperty ffdb p ((getproperty ffdb p)+[10,10,10])
		addmodifier ffdb (bend())
		classof ffdb
		ffdb_c = copy ffdb
		classof ffdb_c
		
		ffdc = spaceFFDCyl radius:50 height:100
		animateAll  ffdc
		with animate on at time 100 for p in (getsubanimnames ffdc.baseobject[#point_controller_container]) do setProperty ffdc p ((getproperty ffdc p)+[10,10,10])
		addmodifier ffdc (bend())
		classof ffdc
		ffdc_c =copy ffdc
		classof ffdc_c
	),

	function FFD_Test testName =
	(
		format "%\n" testName
		filePath = getFileNamePath(getSourceFileName())
		fileName = filePath + "TestData/" + testName + ".max"
		isLoaded = loadMaxFile fileName quiet:true useFileUnits:true
		Assert_true isLoaded message: ("Error " + testName + " (load)")
		if (isLoaded) do
		(
			-- convert to poly so we can compare
			select $Cylinder001
			convertTo $ PolyMeshObject

			-- Compare against our benchmark result
			local vertexList = #()
			local faceList = #()
			local nFaces = $.numFaces
			BuildIndexListOnRange &faceList 1 nFaces 1

			CompareWithBenchmark (testName + "_Benchmark.max") testName 0.0 $.name false &vertexList &faceList 1 true true
		)
	),
	
	function FFD_222_Test =
	(
		FFD_Test ("FFD_222")
	),
	
	function FFD_333_Test =
	(
		FFD_Test ("FFD_333")
	),
	
	function FFD_444_Test =
	(
		FFD_Test ("FFD_444")
	),
	
	function FFD_888_Test =
	(
		FFD_Test ("FFD_888")
	),
	
	function FFD_Box_Test =
	(
		FFD_Test ("FFD_Box")
	),
	
	function FFD_Cyl_Test =
	(
		FFD_Test ("FFD_Cyl")
	),

	Tests =
	#(
		SpaceFFD_animated_cp_with_modifier_test,
		FFD_222_Test,
		FFD_333_Test,
		FFD_444_Test,
		FFD_888_Test,
		FFD_Box_Test,
		FFD_Cyl_Test
	)
)

run_test_fixture FFD_UnitTest script:(getThisScriptFilename())	