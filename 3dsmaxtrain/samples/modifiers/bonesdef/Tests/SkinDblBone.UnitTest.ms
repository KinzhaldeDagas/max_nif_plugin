	/*
    @param TestName:    Skin with same bone listed multiple times
    @param Author:      Abdelhak Ouhlal
    @param Created:     2022-01-14
    @param LastMod:     2022-01-14
    @param ModBy:       Abdelhak Ouhlal
    @param Description: Testing a fix for a skin issue with the same bone listed more than once.
    @param Status:      Online
    @param Tags:
*/

/*
	A model with the skin modifier has some vertices with a list of influencers including duplicated bones.
	On load, the updated code should be able to correct the issue and the duplicates be removed.
	The contributions from the duplicated bones are accumulated onto a single bone.
	
	test_skin_dbl_bones: a test case involving a scene object with Skin modifier showing duplicate bones on the weight table. 
*/

struct SkinDblBoneStruct
(
    private
    test_data_path,
    isLoaded,

    public
    -- setup function: initialization before each unit test
    function setup =
    (
		resetMaxFile #noPrompt		

		filePath = getFileNamePath(getSourceFileName())
		fileName = filePath + "./TestData/SkinWithDoubleEntriesTest.max"
		isLoaded = loadMaxFile fileName quiet:true
		
		print "SETUP Completed"
    ),

    -- teardown function: clean-up step after each unit test
    function teardown =
    (
		resetMaxFile #noPrompt
		print "TEARDOWN Completed"
    ),

	--
	-- test_skin_dbl_bones: a test case involving a scene object with Skin modifier showing duplicate bones on the weight table.
	--
    function test_skin_dbl_bones = 
	(       
		format "Start of test_skin_dbl_bones\n"
		if isLoaded do
		(
			select $Box001
			
			local boneCount, msg
			for i = 1 to $.numverts do
			(
				boneCount =  (skinOps.GetVertexWeightCount $.modifiers[1] i)
				msg = "Error test case: vertex " + (i as string) + " has an invalid number of influence bones: " + (boneCount as string) + "\n"
				assert_true (boneCount < 3) message: (msg)
			)
		)
		format "End of test_skin_dbl_bones\n"
    ),
    	
    -- ==================================================
    -- the array of Tests to run
    -- ==================================================
    Tests = #(
        test_skin_dbl_bones
    ),

    function runAllTests =
    (
        for test in Tests do 
        (
            setup()
            format "Running test %\n" test
            test()
            teardown()
        )
    )
)

-- run the tests:
if undefined == run_test_fixture then
(
    format "-- No fixture!\n"	
    tester = SkinDblBoneStruct()
    tester.runAllTests()
)
else
(
    format "-- Fixture found!\n"
    run_test_fixture SkinDblBoneStruct script:(getThisScriptFilename())
)