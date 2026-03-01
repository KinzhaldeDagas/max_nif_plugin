/*
    @param TestName:       MorpherChnOver100
    @param Author:         Abdelhak Ouhlal
    @param Created:        February 2024
    @param LastMod:        February 2024
    @param ModBy:          February Ouhlal
    @param Description:    Morpher modifier's progressive morphs on channels over 100 tests :
                               WM3_MC_BuildFromNode(),
                               WM3_AddProgressiveMorphNode(),
                               WM3_SetProgressiveMorphWeight()
    @param Status:          Online
*/

fileIn "MxsUnit/MxsUnit.ms"

Struct MorpherChnOver100
(
    private
    test_data_path,
    total_num_channels,
    loadRes,

    public

    function init_data =
    (
        -- initialize some data to compare against
        total_num_channels = 102
    ),

    -- setup function: initialization before each unit test
    Function setup =
    (
        resetMaxFile #noPrompt
        clearListener() 

        format "Setup..\n"        
        local thisFile = getSourceFileName()
        local thisDir = pathConfig.removePathLeaf thisFile
        test_data_path = pathConfig.appendpath thisDir "TestData"
        
        maxscene = pathConfig.appendPath test_data_path "morpher_chn_over100.max"
        loadRes = loadMaxFile maxScene useFileUnits:true quiet:true    

		init_data()
    ),

    -- teardown function: clean-up step after each unit test
    function teardown =
    (
        resetMaxFile #noPrompt
        clearListener() 
        
        print "TEARDOWN Completed"
    ),

    fn test_build_from_node_over_100 =
    (
        format "Beginning of test_build_from_node_over_100\n"

        assert_true loadRes message:("Error loading 2025 test morpher_chn_over100.max file.")
        
		morpherMod = $Box001.Modifiers[1]
		
		for i = 1 to total_num_channels do (
			local bresult = WM3_MC_BuildFromNode morpherMod i $Box002
			format "bresult: %" (bresult as string)
			error_msg = "WM3_MC_BuildFromNode() call failed at channel " + (i as string)
			assert_true bresult message:error_msg
		)

        format "End of test_build_from_node_over_100\n"
    ),

    fn test_add_progressive_over_100 =
    (
        format "Beginning of test_add_progressive_over_100\n"

        assert_true loadRes message:("Error loading 2025 test morpher_chn_over100.max file.")
        
		morpherMod = $Box001.Modifiers[1]
		for i = 1 to total_num_channels do (
			WM3_MC_BuildFromNode morpherMod i $Box002

			local bresult = WM3_AddProgressiveMorphNode morpherMod i $Box003
			error_msg = "WM3_AddProgressiveMorphNode() call failed at channel " + (i as string) + " for Box003"
			assert_true bresult message:error_msg

			bresult = WM3_AddProgressiveMorphNode morpherMod i $Box004
			error_msg = "WM3_AddProgressiveMorphNode() call failed at channel " + (i as string) + " for Box004"
			assert_true bresult message:error_msg
		)
		
        format "End of test_add_progressive_over_100\n"
    ),
	
    fn test_set_progressive_over_100 =
    (
        format "Beginning of test_set_progressive_over_100\n"

        assert_true loadRes message:("Error loading 2025 test morpher_chn_over100.max file.")
        
		morpherMod = $Box001.Modifiers[1]
		for i = 1 to total_num_channels do (
			WM3_MC_BuildFromNode morpherMod i $Box002
			WM3_AddProgressiveMorphNode morpherMod i $Box003
			WM3_AddProgressiveMorphNode morpherMod i $Box004			
		)
		
		for i = 1 to total_num_channels do (
			local bresult = WM3_SetProgressiveMorphWeight morpherMod i $Box004 50.0
			error_msg = "WM3_SetProgressiveMorphWeight() call failed at channel " + (i as string) + " for Box004"
			assert_true bresult message:error_msg
			
			bresult = WM3_SetProgressiveMorphWeight morpherMod i $Box003 31.0
			error_msg = "WM3_SetProgressiveMorphWeight() call failed at channel " + (i as string) + " for Box003"
			assert_true bresult message:error_msg

			bresult = WM3_SetProgressiveMorphWeight morpherMod i $Box002 100.0
			error_msg = "WM3_SetProgressiveMorphWeight() call failed at channel " + (i as string) + " for Box002"
			assert_true bresult message:error_msg
		)

        format "End of test_set_progressive_over_100\n"
    ),	
    -- ==================================================
    -- the array of Tests to run
    -- ==================================================
    Tests = #(
        test_build_from_node_over_100,
        test_add_progressive_over_100,
        test_set_progressive_over_100
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
    tester = MorpherChnOver100()
    tester.runAllTests()
)
else
(
    format "-- Fixture found!\n"
    run_test_fixture MorpherChnOver100 script:(getThisScriptFilename())
)