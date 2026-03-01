--------------------------------------------------------------------------------------
-- Tests for the Relax modifier, covering various settings and inputs.
--
-- Author: Mathew Kaustinen
-- Created: April 2020
--------------------------------------------------------------------------------------

Struct RelaxUnitTests 
( 
	Function setup = 
	(
		resetMaxFile #noPrompt 
	),

	Function teardown = 
	( 
	),

	Function relaxTest1 = 
	(
		format "Relax Test1 - Simple Mesh without Volume Preservation\n"
		testbox = Box width:100.0 length:50.0 height:50.0 pos:[0,0,0] lenghtsegs:2 widthsegs:2 heightsegs:2 isSelected:on

		max modify mode
		addModifier $ (Relax())
		$.modifiers[#Relax].PreserveVolume = off
		$.modifiers[#Relax].value = 0.5
		$.modifiers[#Relax].iterations = 10
		
		convertTo $ PolyMeshObject
		-- Check Vertex Value
		vert = $.GetVertex 1 
		assert_point3_equal [-9.53352,-1.44904,20.2332] vert tolerance:0.0001 message:"Test 1 failed"
	),
	
	Function relaxTest2 = 
	(
		format "Relax Test2 - Simple Mesh with Volume Preservation\n"
		testbox = Box width:100.0 length:50.0 height:50.0 pos:[0,0,0] lenghtsegs:2 widthsegs:2 heightsegs:2 isSelected:on

		max modify mode
		addModifier $ (Relax())
		$.modifiers[#Relax].PreserveVolume = on
		$.modifiers[#Relax].value = 0.5
		$.modifiers[#Relax].iterations = 10
		
		convertTo $ PolyMeshObject
		-- Check Vertex Value
		vert = $.GetVertex 1 
		assert_point3_equal [-41.9478,-13.9935,4.02609] vert tolerance:0.0001 message:"Test 2 failed"
	),
	
	Function relaxTest3 = 
	(
		format "Relax Test3 - Simple Poly without Volume Preservation\n"
		testbox = Box width:100.0 length:50.0 height:50.0 pos:[0,0,0] lenghtsegs:2 widthsegs:2 heightsegs:2 isSelected:on
		convertTo $ PolyMeshObject
		
		max modify mode
		addModifier $ (Relax())
		$.modifiers[#Relax].PreserveVolume = off
		$.modifiers[#Relax].value = 0.5
		$.modifiers[#Relax].iterations = 10
		convertTo $ PolyMeshObject
		-- Check Vertex Value
		vert = $.GetVertex 1 
		assert_point3_equal [-9.53352,-1.44904,20.2332] vert tolerance:0.0001 message:"Test 3 failed"
	),
	
	Function relaxTest4 = 
	(
		format "Relax Test4 - Simple Poly with Volume Preservation\n"
		testbox = Box width:100.0 length:50.0 height:50.0 pos:[0,0,0] lenghtsegs:2 widthsegs:2 heightsegs:2 isSelected:on
		convertTo $ PolyMeshObject
		
		max modify mode
		addModifier $ (Relax())
		$.modifiers[#Relax].PreserveVolume = on
		$.modifiers[#Relax].value = 0.5
		$.modifiers[#Relax].iterations = 10
		
		convertTo $ PolyMeshObject
		-- Check Vertex Value
		vert = $.GetVertex 1 
		assert_point3_equal [-41.9478,-13.9935,4.02609] vert tolerance:0.0001 message:"Test 4 failed"
	),
	 
	Tests  = #(relaxTest1 ,relaxTest2, relaxTest3 ,relaxTest4) 
)

if undefined ==  run_test_fixture  then 
( 
	format "-- No fixture!\n" 
	testList = RelaxUnitTests()
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
	run_test_fixture MeshSmoothUnitTests script:(getThisScriptFilename())
) 
