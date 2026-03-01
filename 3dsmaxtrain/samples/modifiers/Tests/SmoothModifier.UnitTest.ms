--##################################################
-- Modifiers UI simplification Unit Test
-- Author:	Etienne Clement
-- Created:	Nov 21, 2002
--##################################################
-- The following macros are testing the changes
-- made to the modifier UI in Kahn. For more
-- information view spec #1159b
--##################################################
fileIn "MxsUnit/MxsUnit.ms"
fileIn ((getFileNamePath(getSourceFileName())) + "/CompareWithBenchmark.mxs")

struct Obj_Prop_UnitTest
(
private
	--##################################################
	-- Expected values
	--##################################################

	-- Smooth modidifer
	function getSmoothProperties =
	(
		#(false,	\ -- Auto smooth
		 false,	\ -- Prevent indirect
		"30.0",	\ -- Threshold
		0) 		-- Smoothing bits
	),
	testprops = #(
		"autosmooth",
		"preventindirect",
		"threshold",
		"smoothingbits"
	),

	--##################################################
	-- Helper functions
	--##################################################

	function fetchSmoothProperties mod properties =
	(
		append properties mod.autoSmooth
		append properties mod.preventIndirect
		append properties (mod.threshold as string)
		append properties mod.smoothingBits
	),

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

	function createSmooth =
	(
		box1 = box name:#box1
		smooth1 = smoothModifier()
		addModifier box1 smooth1

		index = modPanel.getModifierIndex box1 box1.smooth
		smooth1 = box1.modifiers[index]

		expected = getSmoothProperties()
		experimental = #()
		fetchSmoothProperties smooth1 experimental
		validateProperties experimental expected
		forcecompleteredraw()
	),

	function loadSmooth =
	(
		local box1 = box name:#box1
		local smooth1 = smoothModifier()
		addModifier box1 smooth1

		local box2 = box name:#box2 pos:[20, 20, 20]
		local smooth2 = smoothModifier()
		smooth2.autoSmooth 		= true
		smooth2.preventIndirect 	= true
		smooth2.threshold 		= 15.0
		smooth2.smoothingBits 	= 2
		addModifier box2 smooth2

		saveMaxFile "toload.max"

		resetMaxFile #noPrompt

		loadMaxFile "toload.max"

		box1 = getNodeByName "box1" exact:true
		index = modPanel.getModifierIndex box1 box1.smooth
		smooth1 = box1.modifiers[index]

		local expected = getSmoothProperties()
		experimental = #()
		fetchSmoothProperties smooth1 experimental
		validateProperties experimental expected

		box2 = getNodeByName "box2" exact:true
		index = modPanel.getModifierIndex box2 box2.smooth
		smooth2 = box2.modifiers[index]

		expected = getSmoothProperties()
		changevalue "autosmooth" 	true expected
		changevalue "preventindirect" true expected
		changevalue "threshold" 		(15.0 as string) expected
		changevalue "smoothingbits" 	2 	expected

		experimental = #()
		fetchSmoothProperties smooth2 experimental
		validateProperties experimental expected
		forcecompleteredraw()
	),

	function mergeSmooth =
	(
		box1 = box name:#box1
		smooth1 = smoothModifier()
		addModifier box1 smooth1

		box2 = box name:#box2 pos:[20, 20, 20]
		smooth2 = smoothModifier()
		smooth2.autoSmooth = true
		smooth2.preventIndirect = true
		smooth2.threshold = 15.0
		smooth2.smoothingBits = 2
		addModifier box2 smooth2

		saveMaxFile "tomerge.max"

		resetMaxFile #noPrompt

		loadMaxFile "tomerge.max"

		box1 = getNodeByName "box1" exact:true
		index = modPanel.getModifierIndex box1 box1.smooth
		smooth1 = box1.modifiers[index]

		expected = getSmoothProperties()
		experimental = #()
		fetchSmoothProperties smooth1 experimental
		validateProperties experimental expected

		box2 = getNodeByName "box2" exact:true
		index = modPanel.getModifierIndex box2 box2.smooth
		smooth2 = box2.modifiers[index]

		expected = getSmoothProperties()
		changevalue "autosmooth" true expected
		changevalue "preventindirect" true expected
		changevalue "threshold" (15.0 as string) expected
		changevalue "smoothingbits" 2 expected

		experimental = #()
		fetchSmoothProperties smooth2 experimental
		validateProperties experimental expected
		forcecompleteredraw()
	),

	Function SmoothSelectionTests =
	(
		local nCases = 3
		local nRowsSphere = 16
		local nFacesSphereRow = 32
		local isPolyCase = #(true,false,false)
		local isModifierCase = #(false,false,true)

		-- Test smoothing with and without an input face selection, for both Poly and Tri objects
		-- ======================================================================================
		format "Smooth Modifier -- Smooth Selection Tests\n"

		-- Build an array of polygon faces to be checked in our benchmark comparisons, following along a half-circumference
		-- from the top to the bottom of the sphere
		vertexList = #()
		faceList = #()
		faceList.count = nRowsSphere
		for r = 1 to nRowsSphere do
		(
			faceList[r] = 1 + (r - 1) * nFacesSphereRow
		)

		-- Test subcases with both Poly and Tri input objects
		nCasesLoop = isPolyCase.count
		for c = 1 to nCasesLoop do
		(
			-- Generate a sphere, which by default will have a single shared normal at each vertex, convert to Editable Poly or
			-- Editable Mesh, or add an Edit Mesh modifier, as appropriate for case c, and apply an Edit Normals modifier
			resetMaxFile #noprompt
			sphereCurr = Sphere segs:nFacesSphereRow pos:[0.0,0.0,0.0] isSelected:on radius:25.0 mapCoords:true
			select sphereCurr
			max modify mode
			if (isPolyCase[c]) then
			(
				convertTo $ PolyMeshObject
			)
			else
			(
				if (isModifierCase[c]) then
				(
					addModifier $ (Edit_Mesh())
				)
				else
				(
					convertTo $ TriMeshGeometry
				)
			)

			addModifier $ (EditNormals())

			-- Break all normals on the sphere without averaging, generating a distinct specified normal at each face corner
			normalSelectionFlags = #{}
			nNormalsInit = ($.modifiers[1]).GetNumNormals()
			normalSelectionFlags.count = nNormalsInit
			for n = 1 to nNormalsInit do
			(
				normalSelectionFlags[n] = true
			)

			($.modifiers[1]).SetSelection &normalSelectionFlags
			($.modifiers[1]).Break toAverage:false

			-- Convert to Editable Poly or Editable Mesh, or add an Edit Mesh modifier, as appropriate for case c
			if (isPolyCase[c]) then
			(
				convertTo $ PolyMeshObject
			)
			else
			(
				if (isModifierCase[c]) then
				(
					addModifier $ (Edit_Mesh())
				)
				else
				(
					convertTo $ TriMeshGeometry
				)
			)

			-- Apply a smooth modifier without setting a face selection, set for Auto Smooth
			addModifier $ (smooth autosmooth:true)

			-- Compare with benchmark result
			caseName = (c as string) + "a"
			CompareWithBenchmark ("SmoothSelectionBenchmark" + caseName + ".max") caseName 0.0 sphereCurr.name true &vertexList &faceList 1 false true

			-- Remove smooth modifier
			select sphereCurr
			max modify mode
			deleteModifier $ 1

			-- Select the top hemisphere of sphere sphereCurr
			faceSelectionFlags = #{}
			nFacesSphere = $.numFaces
			faceSelectionFlags.count = nFacesSphere
			nFacesHemisphere = nFacesSphere / 2
			for f = 1 to nFacesHemisphere do
			(
				faceSelectionFlags[f] = true
			)

			for f = (nFacesHemisphere + 1) to nFacesSphere do
			(
				faceSelectionFlags[f] = false
			)

			subobjectLevel = 4
			if (isModifierCase[c]) then
			(
				setFaceSelection $ 1 faceSelectionFlags	
			)
			else
			(
				setFaceSelection $ faceSelectionFlags
			)

			-- Apply a smooth modifier with a face selection, set for Auto Smooth, and compare with benchmark results
			addModifier $ (smooth autosmooth:true)
			if (isModifierCase[c]) then
			(
				modPanel.setCurrentObject $.modifiers[2]
			)
			else
			(
				modPanel.setCurrentObject $.baseObject
			)

			subobjectLevel = 4
			modPanel.setCurrentObject $.modifiers[1]
			caseName = (c as string) + "b"
			CompareWithBenchmark ("SmoothSelectionBenchmark" + caseName + ".max") caseName 0.0 sphereCurr.name false &vertexList &faceList 1 false true
		)
	),

	Tests =
	#(
		createSmooth,
		loadSmooth,
		mergeSmooth,
		SmoothSelectionTests
	)
)
/* Useful for debugging. Do not remove.
foo = Obj_Prop_UnitTest()
foo.setup()
foo.createSmooth()
resetmaxfile(#noprompt)
foo.loadSmooth()
resetmaxfile(#noprompt)
foo.mergeSmooth()
*/
run_test_fixture Obj_Prop_UnitTest script:(getThisScriptFilename())