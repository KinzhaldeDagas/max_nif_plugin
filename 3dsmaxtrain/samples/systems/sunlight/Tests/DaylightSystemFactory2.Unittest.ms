/*
    @param TestName:        DaylightSystemFactory2.UnitTest
    @param Author:          larry.minton@autodesk.com
    @param Created:         
    @param LastMod:         08-28-2024
    @param ModBy:           Larry Minton
    @param Description:     Miscellaneous DaylightSystemFactory2 tests
    @param Status:          Online
*/

fileIn "MxsUnit/MxsUnit.ms"

struct DaylightSystemFactory2_TestFixture
(

	fn setUp =
	(
		resetMaxFile #noprompt 
	),
	
	fn tearDown =
	(
	),
	
	fn test_create = 
	(
		print "running test_create"
		IDaylightSystem = DaylightSystemFactory2.create()
		assert_equal 4 objects.count
		assert_equal Directionallight (classof (IDaylightSystem.GetSun()))
		assert_equal Skylight (classof (IDaylightSystem.GetSky()))
		-- Directionallight is only applicable sunClass
		-- Skylight and IES_Sky are only applicable skyClass
		IDaylightSystem = DaylightSystemFactory2.create sunClass:Directionallight skyClass:IES_Sky
		assert_equal 8 objects.count
		assert_equal Directionallight (classof (IDaylightSystem.GetSun()))
		assert_equal IES_Sky (classof (IDaylightSystem.GetSky()))
	),

	tests = 
	#(
		test_create
	)
		
)

run_test_fixture DaylightSystemFactory2_TestFixture script:(getThisScriptFilename())
