--##################################################
-- Modifiers Edit Spline Unit Test
-- Author:	 Tom Hudson
-- Created:	 September 19, 2023
--##################################################

fileIn "MxsUnit/MxsUnit.ms"

struct EditSpline_UnitTest
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

	function ValidateSpline testName =
	(
		-- Add an Edit Mesh so we can easily validate vertex positions
		modPanel.addModToSelection (Edit_Mesh ()) ui:on
		-- First check vertex count
		assert_equal 40 $.numVerts message:("Test " + testName + "numVerts failure")
		-- Now validate some vertex positions to be sure they're conforming
		assert_point3_equal [0.956179,20.8307,8.24411] (meshop.getvert $ 17) tolerance:0.001 message:("Test " + testName + " vertex 17 error")
		assert_point3_equal [-1.13397,17.5417,3.96164] (meshop.getvert $ 18) tolerance:0.001 message:("Test " + testName + " vertex 18 error")
		assert_point3_equal [-2.68615,13.9672,-0.904202] (meshop.getvert $ 19) tolerance:0.001 message:("Test " + testName + " vertex 19 error")
		assert_point3_equal [-3.65932,10.1938,-2.99377] (meshop.getvert $ 20) tolerance:0.001 message:("Test " + testName + " vertex 20 error")
		assert_point3_equal [-4.03169,6.31457,-2.89206] (meshop.getvert $ 21) tolerance:0.001 message:("Test " + testName + " vertex 21 error")
		assert_point3_equal [-3.80095,2.42437,-1.60829] (meshop.getvert $ 22) tolerance:0.001 message:("Test " + testName + " vertex 22 error")
		assert_point3_equal [-2.96825,-1.38253,-0.231457] (meshop.getvert $ 23) tolerance:0.001 message:("Test " + testName + " vertex 23 error")
		assert_point3_equal [-1.5491,-5.01187,2.07216] (meshop.getvert $ 24) tolerance:0.001 message:("Test " + testName + " vertex 24 error")
		assert_point3_equal [0.419258,-8.37521,4.76248] (meshop.getvert $ 25) tolerance:0.001 message:("Test " + testName + " vertex 25 error")
		assert_point3_equal [2.88512,-11.3929,8.22569] (meshop.getvert $ 26) tolerance:0.001 message:("Test " + testName + " vertex 26 error")
		-- Clean up
		deleteModifier $ 1
	),
	
	function EditSpline =
	(
		--Defect MAXX-75279 Spline with soft selection fails to conform
		filePath = getFileNamePath(getSourceFileName())
		fileName = filePath + "/TestData/Conform_SoftSelection_Bug.max"
		isLoaded = loadMaxFile fileName quiet:true useFileUnits:true
		Assert_true isLoaded message: "Error Test 1 (load)"
		if (isLoaded) do
		(
			Select $Circle001
			-- Scene should be OK right after load
			ValidateSpline "Phase 1a"
			-- Conform should be OK after disable/enable Conform
			$.modifiers[1].enabled = false
			$.modifiers[1].enabled = true
			ValidateSpline "Phase 1b"			
		)
		isLoaded = loadMaxFile fileName quiet:true useFileUnits:true
		Assert_true isLoaded message: "Error Test 2 (load)"
		if (isLoaded) do
		(
			Select $Circle001
			-- Scene should be OK right after load
			ValidateSpline "Phase 2a"
			-- Conform should be OK after parameter change
			$.modifiers[1].selectionAmount = 10
			ValidateSpline "Phase 2b"			
		)
	),

	Tests =
	#(
		EditSpline
	)
)

run_test_fixture EditSpline_UnitTest script:(getThisScriptFilename())
