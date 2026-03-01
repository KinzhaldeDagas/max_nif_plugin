/*
	@param TestName		: RenderElements.UnitTest.ms
	@param Author		: larry.minton@autodesk.com
	@param Created		: 03-16-2020
	@param LastMod		: 
	@param ModBy		: 
	@param Desc			: misc RenderElements tests
	@param Status		: Online
*/

fileIn "MxsUnit/MxsUnit.ms"
struct RenderElementsTests
(
public
	-- Unit test fixture setup
	function Setup =
	(
		resetmaxFile #noPrompt
	),

	-- Unit test fixture teardown
	function TearDown =
	(
		-- add here fixture teardown code
	),
	
	function test_FPS = 
	(
		local sm = specularMap() --> ReferenceTarget:SpecularMap
		sm.backgroundColor = red
		assert_equal red sm.backgroundColor
		assert_equal 2 (sm.nParams())
		assert_string_equal "Lighting" (sm.paramName 1)
    	assert_string_equal "Shadows" (sm.paramName 2)
    	assert_equal 1 (sm.paramType 1)
    	assert_equal 1 (sm.paramType 2)
		assert_equal 0 (sm.getParamValue 1)
		assert_equal 0 (sm.getParamValue 2)
		sm.setParamValue 1 2
		sm.setParamValue 2 3
		assert_equal 2 (sm.getParamValue 1)
		assert_equal 3 (sm.getParamValue 2)
		assert_equal 2 (sm.getParamFPValue 1)
		assert_equal 3 (sm.getParamFPValue 2)
		sm.setParamFPValue 1 4
		sm.setParamFPValue 2 5
		assert_equal 4 (sm.getParamFPValue 1)
		assert_equal 5 (sm.getParamFPValue 2)
		assert_equal -999999999 (sm.getParamFPValueMin 1)
		assert_equal -999999999 (sm.getParamFPValueMin 2)
		assert_equal 999999999 (sm.getParamFPValueMax 1)
		assert_equal 999999999 (sm.getParamFPValueMax 2)
		assert_true (sm.findParam "Lighting")
		assert_true (sm.findParam "Shadows")
	),
	
	Tests = 
	#(
		test_FPS
	)
)

if undefined == run_test_fixture then
(
	format "-- Unit test fixture not found. Running tests outside of fixture.\n"
	TestCase = RenderElementsTests()
	TestCase.Setup()
	TestCase.testAsserts()
	TestCase.Teardown()
)
else
(
	format "-- Fixture found\n"
	run_test_fixture RenderElementsTests script:(getThisScriptFilename())
)