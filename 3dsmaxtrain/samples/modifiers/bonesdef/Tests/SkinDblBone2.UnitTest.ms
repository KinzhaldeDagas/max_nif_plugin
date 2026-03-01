	/*
    @param TestName:    Skin with same bone listed multiple times removes wrong bones
    @param Author:      Abdelhak Ouhlal
    @param Created:     2022-03-16
    @param LastMod:     2022-03-16
    @param ModBy:       Abdelhak Ouhlal
    @param Description: Testing a fix for a skin issue with the same bone listed more than once but was removing the wrong bone.
    @param Status:      Online
    @param Tags:
*/

/*
	A 2022 sample model, from a beta user, with the skin modifier has some vertices with a list of influencers including duplicated bones.
    The issue with the last fix (was using DeleteWeight()) is that it was missing a case where the bones' indices are not ordered: 
    it removes the wrong bone from the list of influencers.
	
	test_skin_dbl_bones_2: a test case involving a scene object with Skin modifier showing duplicate bones on the weight table but after 
                           the previous fix, the wrong bone was removed because we were using the DeleteWeight() function inside a loop 
                           which decreases the indices of the bones and gets not applied for the bones with higher indices.
*/

struct VertexBones (
    public 
    vtxIndex,
    nbrBones
)

struct SkinDblBone2Struct
(
    private
    test_data_path,
    isLoaded,
    vtxBonesArray,

    public
    -- setup function: initialization before each unit test
    function setup =
    (
		resetMaxFile #noPrompt		

		filePath = getFileNamePath(getSourceFileName())
		fileName = filePath + "./TestData/SkinWithDoubleEntriesTest2_2022.max"
		isLoaded = loadMaxFile fileName quiet:true
		vtxBonesArray = #(VertexBones 105 4,
                        VertexBones 106 4,
                        VertexBones 107 4,
                        VertexBones 108 4,
                        VertexBones 109 4,
                        VertexBones 110 4,
                        VertexBones 111 4,
                        VertexBones 112 4,
                        VertexBones 113 4,
                        VertexBones 114 4,
                        VertexBones 115 4,
                        VertexBones 116 4,
                        VertexBones 131 4,
                        VertexBones 133 4,
                        VertexBones 134 4,
                        VertexBones 156 4,
                        VertexBones 188 4,
                        VertexBones 189 4,
                        VertexBones 196 4,
                        VertexBones 197 4,
                        VertexBones 198 4,
                        VertexBones 251 3,
                        VertexBones 252 3,
                        VertexBones 253 3,
                        VertexBones 254 3,
                        VertexBones 256 3,
                        VertexBones 257 3,
                        VertexBones 258 3,
                        VertexBones 260 3,
                        VertexBones 268 3,
                        VertexBones 269 3,
                        VertexBones 270 3,
                        VertexBones 271 3,
                        VertexBones 272 3,
                        VertexBones 273 3,
                        VertexBones 274 3,
                        VertexBones 275 3,
                        VertexBones 276 3,
                        VertexBones 277 3,
                        VertexBones 278 3,
                        VertexBones 279 3,
                        VertexBones 281 3,                        
                        VertexBones 282 3,
                        VertexBones 283 3,
                        VertexBones 284 3,
                        VertexBones 285 3,
                        VertexBones 286 3,
                        VertexBones 287 3,
                        VertexBones 288 3,
                        VertexBones 289 3,
                        VertexBones 290 3,
                        VertexBones 291 3,
                        VertexBones 292 3,
                        VertexBones 293 3,
                        VertexBones 294 3,
                        VertexBones 295 3,
                        VertexBones 296 3,
                        VertexBones 297 3,
                        VertexBones 298 3,
                        VertexBones 299 3,
                        VertexBones 300 3,
                        VertexBones 301 3,
                        VertexBones 302 3,
                        VertexBones 303 3,
                        VertexBones 304 3,
                        VertexBones 305 3,
                        VertexBones 306 3,
                        VertexBones 307 3,
                        VertexBones 308 3,
                        VertexBones 309 3,
                        VertexBones 310 3,
                        VertexBones 311 3,
                        VertexBones 312 3,
                        VertexBones 313 2,
                        VertexBones 314 3,
                        VertexBones 315 3,
                        VertexBones 316 3,
                        VertexBones 317 3,
                        VertexBones 318 3,
                        VertexBones 319 3,
                        VertexBones 320 3,
                        VertexBones 321 3,
                        VertexBones 322 3,
                        VertexBones 323 3,
                        VertexBones 324 3,
                        VertexBones 325 2,
                        VertexBones 326 3,
                        VertexBones 327 3,
                        VertexBones 328 2,
                        VertexBones 329 3
                    )
		print "SETUP Completed"
    ),

    -- teardown function: clean-up step after each unit test
    function teardown =
    (
		resetMaxFile #noPrompt
		print "TEARDOWN Completed"
    ),

	--
	-- test_skin_dbl_bones_2: a test case involving a scene object with Skin modifier showing duplicate bones on 
    -- the weight table but the wrong bones get removed from the list of influencers.
	--
    function test_skin_dbl_bones_2 = 
	(       
		format "Start of test_skin_dbl_bones_2\n"
		if isLoaded do
		(
			select $RA_hair
			
			local boneCount, msg
			for i = 1 to vtxBonesArray.count do
			(
				boneCount =  (skinOps.GetVertexWeightCount $.modifiers[1] vtxBonesArray[i].vtxIndex)
				msg = "Error test case: vertex " + (vtxBonesArray[i].vtxIndex as string) + " has an invalid number of influence bones: " + (boneCount as string) + "\n"
                assert_equal vtxBonesArray[i].nbrBones boneCount message:(msg)
			)
		)
		format "End of test_skin_dbl_bones_2\n"
    ),
    	
    -- ==================================================
    -- the array of Tests to run
    -- ==================================================
    Tests = #(
        test_skin_dbl_bones_2
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
    tester = SkinDblBone2Struct()
    tester.runAllTests()
)
else
(
    format "-- Fixture found!\n"
    run_test_fixture SkinDblBone2Struct script:(getThisScriptFilename())
)