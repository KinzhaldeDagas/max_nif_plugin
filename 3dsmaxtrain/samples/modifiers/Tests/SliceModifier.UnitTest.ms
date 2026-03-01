--##################################################
-- Slice Modifier Unit Test
-- Author:	Tom Hudson
-- Created:	May 5 2021
--##################################################

Struct  SliceUnitTests 
( 
	Function setup = 
	( 
		resetmaxFile #noPrompt
	), 
	Function teardown = 
	( 
	),
	Function sliceModifierTests = 
	(
		-- Create a sphere and apply a slice
		format "Slice Test 1: Create a sphere and slice it\n"
		mySphere = sphere radius:100
		select mySphere
		max modify mode
		
		-- Slice
		addModifier $ (SliceModifier())
		verts = $.numVerts
		faces = $.numFaces
		Assert_equal 146 verts message: "Error Slice Verts test case 1"
		Assert_equal 288 faces message: "Error Slice Faces test case 1"
		
		-- Remove positive parts
		format "Slice Test 2: Remove positive\n"
		$.modifiers[#Slice].Slice_Type = 2
		verts = $.numVerts
		faces = $.numFaces
		Assert_equal 97 verts message: "Error Slice Verts test case 2"
		Assert_equal 160 faces message: "Error Slice Faces test case 2"
			
		-- Cap it
		format "Slice Test 3: Cap the slice\n"
		$.modifiers[#Slice].cap = on
		verts = $.numVerts
		faces = $.numFaces
		Assert_equal 81 verts message: "Error Slice Verts test case 3"
		Assert_equal 158 faces message: "Error Slice Faces test case 3"
		
		-- Animate the sphere segments and test
		sliderTime = 100f
		set animate on
		mySphere.segs = 200
		set animate off
			
		format "Slice Test 4: Animated mesh slice #1\n"
		slidertime = 42f
		verts = $.numVerts
		faces = $.numFaces
		Assert_equal 1979 verts message: "Error Slice Verts test case 4"
		Assert_equal 3954 faces message: "Error Slice Faces test case 4"

		format "Slice Test 5: Animated mesh slice #2\n"
		slidertime = 70f
		verts = $.numVerts
		faces = $.numFaces
		Assert_equal 6561 verts message: "Error Slice Verts test case 5"
		Assert_equal 13118 faces message: "Error Slice Faces test case 5"

	),
	 
	Tests  = #(
		sliceModifierTests
	)
)

if undefined ==  run_test_fixture  then 
( 
	format "-- No fixture!\n" 

	foo =  SliceUnitTests  () 
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
	run_test_fixture  SliceUnitTests  script:(getThisScriptFilename()) 
) 

