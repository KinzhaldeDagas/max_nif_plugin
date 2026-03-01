--##################################################
-- Modifiers SkinWrap Regression Test
-- Author: Peter Watje
-- Created:	March 2020
--##################################################
--##################################################
fileIn "MxsUnit/MxsUnit.ms"

struct Obj_Prop_UnitTest
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
/*
	Regression test for  MAXX_56187
	Bind pose was not getting rebuilt on Reset
*/
	function skinWrap_MAXX_56187 =
	(
		Box pos:[-0.303312,0.489571,0] isSelected:on width:17.3843 length:15.9046 height:17.6161
		Sphere radius:8.33238 pos:[-0.223432,1.14953,0] isSelected:on
		actionMan.executeAction 0 "50001"
		toolMode.coordsys #view
		move $ [0,9.59621,0]
		modPanel.addModToSelection (Skin_Wrap ()) ui:on
		$.modifiers[#Skin_Wrap].meshList = #($Box001)
		modPanel.setCurrentObject $.baseObject
		modPanel.addModToSelection (Bend ()) ui:on
		$.modifiers[#Bend].BendAngle = 90
		subobjectLevel = 0
		modPanel.setCurrentObject $.modifiers[#Skin_Wrap]
		$.modifiers[#Skin_Wrap].weightAllVerts = on
		$.modifiers[#Skin_Wrap].meshDeformOps.fullReset ()

		numP = numPoints $ 
		originalBindPos = #()
		for i = 1 to numP do
		(
			originalBindPos[i] = getPointPos $ i
		)

		$.modifiers[#Skin_Wrap].enabled = false

		for i = 1 to numP do
		(
			p = getPointPos $ i
			assert_point3_equal p  originalBindPos[i] tolerance:0.0001 message:"unmatching bind pose"	  
		)
	),

	
	Tests =
	#(
		skinWrap_MAXX_56187
	)
)

run_test_fixture Obj_Prop_UnitTest script:(getThisScriptFilename())