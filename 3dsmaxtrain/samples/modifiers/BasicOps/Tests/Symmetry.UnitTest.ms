--##################################################
-- Modifiers Symmetry Unit Test
-- Author: Mathew Kaustinen
-- Created: January 2021
--##################################################
--##################################################

struct Symmetry_UnitTest
(
	--##################################################
	-- Unit tests
	--##################################################
public
	function setup =
	(
	),

	function teardown =
	(

	),

	/*
		Regression test for files created before MAX 2022 to ensure welding
		result remains the same.
	*/
	function Symmetry_Test1 =
	(
		resetMaxFile #noprompt
		filePath = getFileNamePath (getSourceFileName())
		fileName = filePath + "/TestData/Symmetry_Test_Legacy.max"
		res = loadMaxFile fileName quiet:true
		assert_true res message: "Error loading file - Symmetry_Test1"
		
		select $Sample_Poly
			
		-- Check face and vert count
		local numFaces = $.numFaces
		Assert_equal numFaces 256 message: "Symmetry Error - Bad Face Count. Symmetry_Test1"
		local numVerts = $.numVerts
		Assert_equal numVerts 273 message: "Symmetry Error - Bad Vert Count. Symmetry_Test1"		
		
		-- validate first vert of each face
		local faceFirstVert = #(15,79,85,80,129,12,2,123,11,87,133,114,22,20,19,122,18,19,20,21,7,23,55,123,83,28,30,91,89,92,131,32,33,125,31,95,223,114,40,43,45,127,49,50,51,52,42,23,55,128,101,28,70,69,118,107,72,74,109,76,73,119,70,69,116,103,72,74,105,76,73,117,80,85,86,87,79,78,121,122,88,83,84,93,92,89,90,95,91,98,126,125,96,101,102,94,108,81,110,99,104,107,106,109,120,114,127,113,111,119,118,117,130,121,166,120,123,132,124,134,126,128,129,122,131,125,133,127,135,139,142,140,147,151,152,156,138,144,163,167,171,175,177,179,183,177,175,174,189,191,192,156,162,184,197,199,202,200,207,211,213,216,198,204,223,167,228,230,231,232,236,238,239,240,229,191,192,193,222,184,242,244,246,248,250,251,253,255,252,257,242,244,263,264,250,251,269,255,252,273,140,142,141,144,139,146,159,179,143,162,161,196,200,202,201,204,199,206,235,216,203,222,221,195,249,148,254,208,265,248,270,253,182,167,232,227,164,257,246,273,150,159,166,182,156,210,219,226,235,193,147,179,207,216,163,232)
		
		for index = 1 to numFaces do
		(
			faceIndices = polyop.getFaceVerts $ index
			Assert_equal faceIndices[1] faceFirstVert[index] message:("Symmetry Error - Bad Face Vert Index (Face: " + (index as String) + "). Symmetry_Test1")
		)
		
		-- validate the first 5 vertex values
		local  first5Verts = #([-13.1048,-31.1981,14.8962],[-13.1048,-31.3477,13.8326],[-13.1048,-30.8074,15.4455],[-13.1048,-31.3672,12.3985],[-13.1048,-31.252,8.7024])
		for index = 1 to 5 do
		(
			vert = polyop.getVert $ index
			Assert_point3_equal first5Verts[index] vert tolerance:0.0001 message:("Symmetry Error - Vert Index for vertex: " + (index as String) + ". Symmetry_Test2")
		)
	),
 	

	/*
		Regression test Symmetry operations including welding.
	*/
	function Symmetry_Test2 =
	(
		resetMaxFile #noprompt
		filePath = getFileNamePath (getSourceFileName())
		fileName = filePath + "/TestData/Symmetry_Test_New.max"
		res = loadMaxFile fileName quiet:true
		assert_true res message: "Error loading file - Symmetry_Test2"
			
		select $Sample_Poly
			
		-- Check face and vert count
		local numFaces = $.numFaces
		Assert_equal numFaces 256 message: "Symmetry Error - Bad Face Count. Symmetry_Test2"
		local numVerts = $.numVerts
		Assert_equal numVerts 273 message: "Symmetry Error - Bad Vert Count. Symmetry_Test2"
		
		-- validate first vert of each face
		local faceFirstVert = #(15,81,87,82,131,12,2,125,11,89,135,116,22,20,19,124,18,19,20,21,7,23,55,125,85,28,30,93,91,94,133,32,33,127,31,97,223,116,40,43,45,129,49,50,51,52,42,23,55,130,103,28,72,71,120,109,74,76,111,78,75,121,72,71,118,105,74,76,107,78,75,119,82,87,88,89,81,80,123,124,90,85,86,95,94,91,92,97,93,100,128,127,98,103,104,96,110,83,112,101,106,109,108,111,122,116,129,115,113,121,120,119,132,123,167,122,125,134,126,136,128,130,131,124,133,127,135,129,137,141,144,142,149,153,154,158,140,146,165,168,171,175,177,179,183,177,175,174,189,191,192,158,164,184,197,199,202,200,207,211,213,216,198,204,223,168,228,230,231,232,236,238,239,240,229,191,192,193,222,184,242,244,246,248,250,251,253,255,252,257,242,244,263,264,250,251,269,255,252,273,142,144,143,146,141,148,161,179,145,164,163,196,200,202,201,204,199,206,235,216,203,222,221,195,249,150,254,208,265,248,270,253,182,168,232,227,166,257,246,273,152,161,167,182,158,210,219,226,235,193,149,179,207,216,165,232)
		for index = 1 to numFaces do
		(
			faceIndices = polyop.getFaceVerts $ index
			Assert_equal faceIndices[1] faceFirstVert[index] message:("Symmetry Error - Bad Face Vert Index (Face: " + (index as String) + "). Symmetry_Test2")
		)
		
		-- validate the first 5 vertex values
		local  first5Verts = #([-13.1048,-31.1981,14.8962],[-13.1048,-31.3477,13.8326],[-13.1048,-30.8074,15.4455],[-13.1048,-31.3672,12.3985], [-13.1048,-31.252,8.7024])
		for index = 1 to 5 do
		(
			vert = polyop.getVert $ index
			Assert_point3_equal first5Verts[index] vert tolerance:0.0001 message:("Symmetry Error - Vert Index for vertex: " + (index as String) + ". Symmetry_Test2")
		)
	),
	
	/*
		Regression test for maxx-63339
	*/
	function Symmetry_Slice_Along_The_Mirror_Test =
	(
        resetMaxFile #noprompt
		
        Box typeinCreationMethod:1 typeInLength:1 length:1 width:1 height:1 pos:[0,0,0] isSelected:on
		
        modPanel.addModToSelection (Edit_Poly ()) ui:on
        modPanel.addModToSelection (symmetry PlanarX:false PlanarZ:true) ui:on
			
        select $Box001
        subobjectLevel = 1
        modPanel.setCurrentObject $.modifiers[#Symmetry]		
		format "-- Process symmetry radial with 6 symmetry planes and slice along the mirror on!\n" 			
        $.modifiers[#Symmetry].SymmetryFormat = 1
	    $.modifiers[#Symmetry].RadialCount = 6
        $.modifiers[#Symmetry].RadialMirror = on
			
		local numVerts = $.numVerts
		Assert_equal 26 numVerts message: "Symmetry Slice Along Mirror error"
		
		format "-- Process symmetry with UseProximityThreshold on!\n"
		$.modifiers[#Symmetry].UseProximityThreshold = on
		local numVertsAfterProximityWeld = $.numVerts
		
		Assert_equal numVerts numVertsAfterProximityWeld message: "Symmetry Slice Along Mirror error with proximity weld"
		
		format "-- Process symmetry without welding\n"
		$.modifiers[#Symmetry].weld = 0
		local numVertsWithoutWelding = $.numVerts
		Assert_equal 72 numVertsWithoutWelding  message: "Symmetry Slice Along Mirror error without welding"
		
		format "-- Process symmetry and add a Vertex_Weld modifier\n"
		modPanel.addModToSelection (Vertex_Weld ()) ui:on
		local numVertsWithVertexWeldModifier = $.numVerts
		Assert_equal numVertsWithVertexWeldModifier numVertsAfterProximityWeld message: "Symmetry Slice Along Mirror error"
	),
	
	/*
		Regression test for MAXX-63386
	*/
	function Symmetry_respect_elements_Test =
	(
		resetMaxFile #noprompt
		format "-- Create 2 boxes\n"
		Box pos:[0,0,0] isSelected:on width:10 length:12 height:4
		Box pos:[0,0,4] isSelected:on width:10 length:12 height:4
		select $Box001
		macros.run "Modifier Stack" "Convert_to_Poly"
		
		format "-- Attach them\n"
		$.EditablePoly.attach $Box002 $
		
		local numVerts = $.numVerts
		Assert_equal 16 numVerts message: "Attach operation error"
		
		format "-- Apply a symmetry without welding\n"
		modPanel.addModToSelection (symmetry ()) ui:on
		$.modifiers[#Symmetry].PlanarX = off
		$.modifiers[#Symmetry].PlanarY = on
		$.modifiers[#Symmetry].PlanarZ = off
		$.modifiers[#Symmetry].weld = 0
		
		modPanel.addModToSelection (Edit_Poly ()) ui:on
		modPanel.setCurrentObject $.modifiers[#Edit_Poly]
		local numVertsAfterSymmetry = $.numVerts
		Assert_equal  32 numVertsAfterSymmetry message: "symmetry operation error"
		
		format "-- Apply a symmetry with welding\n"
		$.modifiers[#Symmetry].weld = 1
		local numVertsAfterWeld = $.numVerts
		Assert_equal  26 numVertsAfterWeld message: "symmetry weld error"
	),
	Tests =
	#(
		Symmetry_Test1,
		Symmetry_Test2,
		Symmetry_Slice_Along_The_Mirror_Test,
		Symmetry_respect_elements_Test
	)
)

if undefined ==  run_test_fixture  then 
( 
	format "-- No fixture!\n" 
	testList = Symmetry_UnitTest()
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
	run_test_fixture Symmetry_UnitTest script:(getThisScriptFilename())
) 

