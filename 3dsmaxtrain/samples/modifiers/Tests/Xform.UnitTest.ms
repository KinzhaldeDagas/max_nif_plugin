/*
@param TestName:      Xform
@param Author:          camille.martin@autodesk.com
@param Created:         04/02/2019
@param LastMod:        04/02/2019
@param ModBy:           Camille Martin
@param Description:   Test Reset Xform Modifier on mirrored objects
@param Status:          Online

Unit Test For Xform Modifier
*/

struct XformModifierAutomation
(
	function setup = 
	(
		-- Preventive Cleanup
		resetmaxfile #noprompt
		
		-- Begin test
		format "*******************************************************************************\n"
		format " Xform Modifier basic functionalities testing:\n"
		format "\t- Reset Xform on mirrored objects and toggle the \"Preserve Normals\" ON and OFF\n"
		format "*******************************************************************************\n"
	),
	
	-- Testing preservation of normals on reset Xform for mirrored objects
	function test_preserveNormals =
	(
		format "Setting up test scene...\n"
		
		-- Preventive Cleanup
		resetmaxfile #noprompt
		
		-- Create a teapot
		pot = teapot radius:25 segs:8
			
		-- Convert to a mesh to extract its geometry information
		convertToMesh pot
		
		-- Mirror the teapot
		scale pot [-1, 1, 1]
		
		-- Get its normals
		nbFaces = pot.numFaces
		normalsArray = #()
		for f = 1 to nbFaces do
		(
			n = getFaceNormal pot f
			append normalsArray n
		)
		
		-- Reset Xform for the teapot
		resetxform pot
		
		-- Compare new normals to originals, when 'preserve normals' is on (default)
		format "Testing normals with 'preserve normals' ON\n"
		for f = 1 to nbFaces do
		(
			n = getFaceNormal pot f
			dp = dot n normalsArray[f]
			if dp != 0.0 then
			(	
				assert_float_equal 1.0 dp tolerance:0.0001 message:("Invalid normal for resetted Xform for face #" + f as string + ".") 
			)
		)
		
		-- Test 'preserve normals' off
		format "Testing normals with 'preserve normals' OFF\n"
		pot.modifiers[1].PreserveNormals = false
		for f = 1 to nbFaces do
		(
			n = getFaceNormal pot f
			dp = dot n normalsArray[f]
			if dp != 0.0 then
			(
				assert_float_equal -1.0 dp tolerance:0.0001 message: ("Invalid normals for resetted Xform #" + f as string + ".") 
			)
		)
		
		format "Test completed\n"
	),
	
	function test = 
	(
		format "- Reset Xform on mirrored objects and Toggle the \"Preserve Normals\" ON and OFF\n"
		test_preserveNormals()
	),

	function teardown = 
	(	
		-- End of Xform Modifier testing
		format "*******************************************************************************\n"
	),
	
	Tests = #(test)
)

if undefined == run_test_fixture then
(
	format "-- No fixture!\n"
	foo = XformModifierAutomation()
	foo.setup()
	foo.test()
	foo.teardown()
)
else
(
	format "-- Fixture found!\n"
	run_test_fixture XformModifierAutomation script:(getThisScriptFilename())
)
