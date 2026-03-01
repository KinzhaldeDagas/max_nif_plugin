/*
@param TestName:      UVW Map modifier
@param Author:          Peter Watje`
@param Created:         1/27/2023
@param LastMod:        1/27/2023
@param ModBy:           
@param Description:    Test UVW Map 
@param Status:          Online

Unit Test For UVW Map Modifier
*/

struct UVWMapModifierAutomation
(
	function setup = 
	(
		-- Preventive Cleanup
		resetmaxfile #noprompt
		
		-- Begin test
		format "*******************************************************************************\n"
		format " UVW Map Modifier basic functionalities testing:\n"
		format "*******************************************************************************\n"
	),
	
	-- Testing that the UVW Map modifier does not clear Explicit Normals which it was doing
	function test_ExplicitNormals =
	(
		Box pos:[0,0,0] isSelected:on width:20 length:20 height:20
		modPanel.addModToSelection (Edit_Normals ()) ui:on
		modPanel.setCurrentObject $.modifiers[#Edit_Normals]
		subobjectLevel = 1
		$.modifiers[#Edit_Normals].EditNormalsMod.Select #{1..24}		
		$.modifiers[#Edit_Normals].EditNormalsMod.MakeExplicit ()
		$.modifiers[#Edit_Normals].EditNormalsMod.Unify ()			
		modPanel.addModToSelection (Uvwmap ()) ui:on
		modPanel.addModToSelection (Edit_Normals ()) ui:on
		normMod = $.modifiers[1]
		numNormals = normMod.GetNumNormals ()
		assert_equal 8 numNormals
	),
	

	function teardown = 
	(	
		-- End of UVW Map Modifier testing
		format "*******************************************************************************\n"
	),
	
	Tests = #(test_ExplicitNormals)
)

if undefined == run_test_fixture then
(
	format "-- No fixture!\n"
	foo = UVWMapModifierAutomation()
	foo.setup()
	foo.test_ExplicitNormals()
	foo.teardown()
)
else
(
	format "-- Fixture found!\n"
	run_test_fixture UVWMapModifierAutomation script:(getThisScriptFilename())
)
