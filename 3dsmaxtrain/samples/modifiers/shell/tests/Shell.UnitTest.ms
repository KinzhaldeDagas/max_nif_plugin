/*
@param TestName:      Shell
@param Author:        Peter Watje
@param Created:       04/14/2023
@param LastMod:       02/23/2024
@param ModBy:         Lee Betchen
@param Description:   Test Shell modifier
@param Status:        Online

Unit Test For Shell Modifier
*/

--if rebase is set the script will print the correct values of the mesh
--the info will be printed in the Macro Recorder output
--just paste it over the below values
rebaseData = false


baseMapIndices = #() 
edgeMapIndices = #() 
-- Shell Map Test edge type 0 
baseMapIndices[1] = #(#(10,7,11),#(8,11,7),#(11,8,12),#(9,12,8),#(13,10,14),#(11,14,10),#(14,11,15),#(12,15,11),#(26,22,25),#(22,26,23),#(27,23,26),#(23,27,24),#(29,25,28),#(25,29,26),#(30,26,29),#(26,30,27))
edgeMapIndices[1] = #(#(7,10,10),#(10,7,7),#(8,7,7),#(7,8,8),#(12,9,9),#(9,12,12),#(9,8,8),#(8,9,9),#(10,13,13),#(13,10,10),#(13,14,14),#(14,13,13),#(14,15,15),#(15,14,14),#(15,12,12),#(12,15,15))
-- Shell Map Test edge type 1 
baseMapIndices[2] = #(#(10,7,11),#(8,11,7),#(11,8,12),#(9,12,8),#(13,10,14),#(11,14,10),#(14,11,15),#(12,15,11),#(26,22,25),#(22,26,23),#(27,23,26),#(23,27,24),#(29,25,28),#(25,29,26),#(30,26,29),#(26,30,27))
edgeMapIndices[2] = #(#(1,1,1),#(1,1,1),#(1,1,1),#(1,1,1),#(1,1,1),#(1,1,1),#(1,1,1),#(1,1,1),#(1,1,1),#(1,1,1),#(1,1,1),#(1,1,1),#(1,1,1),#(1,1,1),#(1,1,1),#(1,1,1))
-- Shell Map Test edge type 2 
baseMapIndices[3] = #(#(10,7,11),#(8,11,7),#(11,8,12),#(9,12,8),#(13,10,14),#(11,14,10),#(14,11,15),#(12,15,11),#(26,22,25),#(22,26,23),#(27,23,26),#(23,27,24),#(29,25,28),#(25,29,26),#(30,26,29),#(26,30,27))
edgeMapIndices[3] = #(#(45,47,48),#(48,46,45),#(43,45,46),#(46,44,43),#(39,41,42),#(42,40,39),#(41,43,44),#(44,42,41),#(31,33,34),#(34,32,31),#(33,35,36),#(36,34,33),#(35,37,38),#(38,36,35),#(37,39,40),#(40,38,37))
-- Shell Map Test edge type 3 
baseMapIndices[4] = #(#(10,7,11),#(8,11,7),#(11,8,12),#(9,12,8),#(13,10,14),#(11,14,10),#(14,11,15),#(12,15,11),#(26,22,25),#(22,26,23),#(27,23,26),#(23,27,24),#(29,25,28),#(25,29,26),#(30,26,29),#(26,30,27))
edgeMapIndices[4] = #(#(7,10,31),#(31,32,7),#(8,7,32),#(32,34,8),#(12,9,35),#(35,36,12),#(9,8,34),#(34,35,9),#(10,13,39),#(39,31,10),#(13,14,41),#(41,39,13),#(14,15,43),#(43,41,14),#(15,12,36),#(36,43,15))
	
struct ShellModifierAutomation
(
	
	function setup = 
	(
		-- Preventive Cleanup
		resetmaxfile #noprompt
	),
	
	-- Testing preservation of normals on reset Xform for mirrored objects
	function EdgeMapTest edgeMapType =
	(
		format "-- Shell Map Test edge type % \n" edgeMapType
		Plane length:34.0438 width:35.3296 pos:[-2.91659,-4.28104,0] isSelected:on
		max modify mode
		$.lengthsegs = 2
		$.widthsegs = 2
		modPanel.addModToSelection (Shell ()) ui:on
		$.modifiers[#Shell].edgeMapping = edgeMapType
		macros.run "Modifier Stack" "Convert_to_Mesh"
		--check first 16 uvs tris
		numMapVerts = meshop.getNumMapVerts $ 1
			
		if (rebaseData == true) then
		(
			
			(				
				format "baseMapIndices[%] = #(" (edgeMapType+1)
				for i = 1 to 16 do
				(
					
					currentFaceIndices = meshop.getMapFace $ 1 i
					a = currentFaceIndices[1] as integer
					b = currentFaceIndices[2] as integer
					c = currentFaceIndices[3] as integer
					if (i == 16) then
					(
						format "#(%,%,%)"  a b c
					)
					else
					(
						format "#(%,%,%),"  a b c
					)
				)
				format ")\n"
				
				format "edgeMapIndices[%] = #(" (edgeMapType+1)
				for i = 17 to 32 do
				(
					
					currentFaceIndices = meshop.getMapFace $ 1 i
					a = currentFaceIndices[1] as integer
					b = currentFaceIndices[2] as integer
					c = currentFaceIndices[3] as integer
					if (i == 32) then
					(
						format "#(%,%,%)"  a b c
					)
					else
					(
						format "#(%,%,%),"  a b c
					)
				)
				format ")\n"				
			)
		)
		else
		(		
			-- will vary based on edge map type but must be at least 18
			assert_true(numMapVerts >= 18)
			numFaces = meshop.getNumFaces $
			assert_true(numFaces >= 32)
			for i = 1 to 16 do
			(					
				currentFaceIndices = meshop.getMapFace $ 1 i
				for j = 1 to 3 do
				(
					testVal = currentFaceIndices[j]
					assert_true(baseMapIndices[edgeMapType+1][i][j] == testVal)
					
				)
			)
			for i = 17 to 32 do
			(
				currentFaceIndices = meshop.getMapFace $ 1 i		
				for j = 1 to 3 do
				(
					testVal = currentFaceIndices[j]
					assert_true(edgeMapIndices[edgeMapType+1][i-16][j] == testVal)
					
				)				
			)				
			
		)
	),
	
	function MapTest = 
	(
		-- Begin test
		format "*******************************************************************************\n"
		format " Shell Modifier basic functionalities testing:\n"
		format "*******************************************************************************\n"
		if (rebaseData) then
		(
			clearlistener()
			format "--COPY THIS OVER THE TOP OF CHECK DATA IN THE TEST \n"
			format "baseMapIndices = #() \n"
			format "edgeMapIndices = #() \n"
		)

		EdgeMapTest 0
		EdgeMapTest 1
		EdgeMapTest 2
		EdgeMapTest 3
	),

	Function SaveLoadTest = 
	(
		local nSegments = 5
		local lengthScale = 50.0
		local shellHeight = 5.0

		-- Test preservation of parameters defined in the Modifier base class through a save/load operation;
		-- previously, these parameters were not properly saved by the Shell modifier, so that the modifier's active
		-- state was not retained
		format "Test preservation of base Modifier parameters through save and load\n"
		resetMaxFile #noprompt
		
		-- Build a plane
		local planeCurr = Plane lengthsegs:nSegments widthsegs:nSegments pos:[0.0,0.0,0.0] length:lengthScale width:lengthScale isSelected:on
		select planeCurr
		max modify mode

		-- Apply a Shell modifier to plane planeCurr, and deactivate the modifier
		addModifier $ (Shell outerAmount:shellHeight)
		($.modifiers[1]).enabled = false

		-- Save a temporary file
		local isSaved = false
		local filePath = GetDir #temp
		local fileName = filePath + "/TempFile_ShellSaveLoadTest.max"
		local isNameUnavailable = isMaxFile fileName
		if (not isNameUnavailable) do
		(
			isSaved = saveMaxFile fileName quiet:true
		)

		-- Proceed only if save succeeded
		assert_true isSaved message: "Error in save/load test (save)"
		if (isSaved) do
		(
			-- Load the file
			resetmaxFile #noPrompt
			local isLoaded = loadMaxFile fileName quite:true

			-- If load succeeded, check to ensure that the shell modifier is deactivated on load, consistent with the
			-- state of the model at save time
			assert_true isLoaded message: "Error in save/load test (load)"
			if (isLoaded) do
			(
				select $Plane001
				assert_false ($.modifiers[1]).enabled message: "Error in save/load test (shell modifier active state retention)"
				resetmaxFile #noPrompt
			)

			-- Delete temporary saved file
			deleteFile fileName
		)
	),

	function teardown = 
	(	
	),
	
	Tests = #(MapTest,
		SaveLoadTest)
)

if undefined == run_test_fixture then
(
	format "-- No fixture!\n"
	foo = ShellModifierAutomation()
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
	run_test_fixture ShellModifierAutomation script:(getThisScriptFilename())
)
