/*
    @param TestName:       Morpher.UnitTest2
    @param Author:         Abdelhak Ouhlal
    @param Created:        August 2023
    @param LastMod:        August 2023
    @param ModBy:          Abdelhak Ouhlal
    @param Description:    Morpher modifier's new interfaces IMorpher and IMorpherChannel tests :
                               IMorpher:
                                   IsValid(),
                                   NumChannels(),
                                   GetChannel()
                               IMorpherChannel:
                                   IsProgressive(),
                                   NumTargets(),
                                   GetBaseDeltas(),
                                   GetProgressiveTargetDeltas(),
                                   GetAnimationKeysData()
    @param Status:          Online
*/

fileIn "MxsUnit/MxsUnit.ms"

Struct MorpherInterfacesStruct
(
    private
    test_data_path,
    total_num_channels,
    loadRes,

    public

    function init_data =
    (
        -- initialize some data to compare against
        total_num_channels = 2
    ),

    -- setup function: initialization before each unit test
    Function setup =
    (
        resetMaxFile #noPrompt

        format "Setup..\n"        
        local thisFile = getSourceFileName()
        local thisDir = pathConfig.removePathLeaf thisFile
        test_data_path = pathConfig.appendpath thisDir "TestData"
        
        maxscene = pathConfig.appendPath test_data_path "box_morpher_prog.max"
        loadRes = loadMaxFile maxScene useFileUnits:true quiet:true        
    ),

    -- teardown function: clean-up step after each unit test
    function teardown =
    (
        resetMaxFile #noPrompt
        
        print "TEARDOWN Completed"
    ),

    fn test_morpher_general =
    (
        format "Beginning of test_morpher_general\n"

        assert_true loadRes message:("Error loading 2023 test box_morpher_prog.max file.")
        
        total_num_channels = 100
        select $Box001  
        numChannels = WM3_NumberOfChannels $.modifiers[#morpher]
        assert_equal total_num_channels numChannels message:"Not the number expected for the morpher number of channels" 

        bresult = wm3_IsProgressive $.modifiers[#morpher] 1
        assert_false bresult message:"Channel 1 does not have progressive morphs!"

        bresult = wm3_IsProgressive $.modifiers[#morpher] 2
        assert_true bresult message:"Channel 2 does have progressive morphs!"

        format "End of test_morpher_general\n"
    ),

    fn test_morpher_base_deltas =
    (
        format "Beginning of test_morpher_base_deltas\n"

        assert_true loadRes message:("Error loading 2023 test box_morpher_prog.max file.")
        
        -- using a set of selected vertices, we compare deltas of channel N base morph vs the difference
        -- between base morph and host object geometries.
        -- Channel 1:
        
        ichannel = 1
        ind = 0
        morph_base_vlst = #{82..162, 355} as array

        select $Box001
        ipos = $.position
        $.position = [0,0,0]
        host_base_v = polyop.getverts $ morph_base_vlst
        $.position = ipos
        
        select $Box002
        ipos = $.position
        $.position = [0,0,0]
        morph_base_v = polyop.getverts $ morph_base_vlst
        $.position = ipos
        
        select $Box001
        morph_deltas = wm3_GetBaseDeltas $.modifiers[#morpher] ichannel
        for i =  1 to morph_base_vlst.count do 
        (
            v_expected_delta = morph_base_v[i] - host_base_v[i] 
            ind = morph_base_vlst[i]
            v_current_delta = morph_deltas[ind]
            error_msg = "Channel " + (ichannel as string) + ": Wrong vertex delta for index" + (morph_base_vlst[i] as string)
            assert_point3_equal v_expected_delta v_current_delta tolerance: 0.001 message: error_msg
        )

        -- Channel 2:
        -------------
        ichannel = 2
        morph_base_vlst = #{1..9, 82..90, 163..171, 195..203, 227..235, 259..267, 291..299, 323..331, 355..363} as array

        select $Box001
        ipos = $.position
        $.position = [0,0,0]
        host_base_v = polyop.getverts $ morph_base_vlst
        $.position = ipos

        select $Box003
        ipos = $.position
        $.position = [0,0,0]
        morph_base_v = polyop.getverts $ morph_base_vlst
        $.position = ipos
        
        select $Box001
        morph_deltas = wm3_GetBaseDeltas $.modifiers[#morpher] ichannel        
        for i =  1 to morph_base_vlst.count do 
        (            
            v_expected_delta = morph_base_v[i] - host_base_v[i] 
            ind = morph_base_vlst[i]
            v_current_delta = morph_deltas[ind]
            error_msg = "Channel " + (ichannel as string) + ": Wrong vertex delta for index" + (ind as string)
            assert_point3_equal v_expected_delta v_current_delta tolerance: 0.001 message: error_msg
        )

        format "End of test_morpher_base_deltas\n"
    ),

    fn test_morpher_progressive_deltas =
    (
        format "Beginning of test_morpher_progressive_deltas\n"

        assert_true loadRes message:("Error loading 2023 test box_morpher_prog.max file.")
        
        -- using a set of selected vertices, we compare deltas of target morph M and channel N base morph vs the difference
        -- between morph target and host object geometries.
        --
        -- Channel = 2 and Morph Target = 1:

        ichannel = 2
        imorph_target = 1
        ind = 0
        morph_base_vlst = #{73..81, 154..162, 179..187, 211..219, 243..251, 275..283, 307..315, 339..347, 371..379} as array
        
        select $Box001
        ipos = $.position
        $.position = [0,0,0]
        host_base_v = polyop.getverts $ morph_base_vlst
        $.position = ipos

        select $Box004
        ipos = $.position
        $.position = [0,0,0]
        morph_target_v = polyop.getverts $ morph_base_vlst
        $.position = ipos
        
        select $Box001
        morph_deltas = wm3_GetProgressiveTargetDeltas $.modifiers[#morpher] ichannel imorph_target        
        for i =  1 to morph_base_vlst.count do 
        (            
            v_expected_delta = morph_target_v[i] - host_base_v[i] 
            ind = morph_base_vlst[i]
            v_current_delta = morph_deltas[ind]
            error_msg = "Channel " + (ichannel as string) + ", morph target " + (morph_target as string) + ": Wrong vertex delta for index" + (morph_base_vlst[i] as string)
            assert_point3_equal v_expected_delta v_current_delta tolerance: 0.001 message: error_msg
        )

        -- Channel = 2 and Morph Target = 2:

        ichannel = 2
        imorph_target = 2
        morph_base_vlst = #{73..81, 154..162, 179..187, 211..219, 243..251, 275..283, 307..315, 339..347, 371..379} as array
        
        select $Box001
        ipos = $.position
        $.position = [0,0,0]
        host_base_v = polyop.getverts $ morph_base_vlst
        $.position = ipos

        select $Box005
        ipos = $.position
        $.position = [0,0,0]
        morph_target_v = polyop.getverts $ morph_base_vlst
        $.position = ipos
        
        select $Box001
        morph_deltas = wm3_GetProgressiveTargetDeltas $.modifiers[#morpher] ichannel imorph_target 
        for i =  1 to morph_base_vlst.count do 
        (            
            v_expected_delta = morph_target_v[i] - host_base_v[i] 
            ind = morph_base_vlst[i]
            v_current_delta = morph_deltas[ind]
            error_msg = "Channel " + (ichannel as string) + ", morph target " + (morph_target as string) + ": Wrong vertex delta for index" + (morph_base_vlst[i] as string)
            assert_point3_equal v_expected_delta v_current_delta tolerance: 0.001 message: error_msg
        )
        format "End of test_morpher_progressive_deltas\n"
    ),

    -- ==================================================
    -- the array of Tests to run
    -- ==================================================
    Tests = #(
        test_morpher_general,
        test_morpher_base_deltas,
        test_morpher_progressive_deltas
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
    tester = MorpherInterfacesStruct()
    tester.runAllTests()
)
else
(
    format "-- Fixture found!\n"
    run_test_fixture MorpherInterfacesStruct script:(getThisScriptFilename())
)