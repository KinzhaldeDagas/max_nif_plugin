/*
	@param TestName		: MxsUnitAssert.UnitTest.ms
	@param Author		: chris.p.johnson@autodesk.com
	@param Created		: 07-20-2010
	@param LastMod		: 11-1-2022
	@param ModBy		: Larry Minton
	@param Desc			: Tests assert methods.
	@param Status		: Online
*/

fileIn "MxsUnit/MxsUnit.ms"
struct MxsDiagnosticAssertTests
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
	
	function testAsserts = 
	(
		AssertReporter.Clear()
		
		-- =========================================================
		-- Test basic true/false tests
		assert_true true
		assert_false ("hh" == "hi")
		assert_false false
		
		val = 5
		assert_equal 5 val message:"These numbers should be the same"

		-- =========================================================
		-- Test Equal methods
		h = bitmap 40 40
		assert_equal h h             message:"These bitmaps should be the same"
		
		-- =========================================================
		-- Test Not Equal methods
		assert_not_equal "sdastas" h message:"These are not the same"
	
	
		-- =========================================================
		-- Test defined functions
		assert_defined   h     message:"These should be defined"
		assert_undefined chris message:"Chris should be undefined"
		
		-- =========================================================
		-- Test Float comparision functions
		assert_float 5.6 5.6
		local val = 9.8925632456
		assert_not_equal 5.6 val 
		
		-- =========================================================
		-- Test String comparison
		-- Default is to ignore case
		assert_string_equal "chris" "chris"
		assert_not_equal "chris" "Joe" 
		assert_string_equal "jack" "jack" ignorecase:false
		assert_string_equal "Hannah" "HANNAH"
		
		-- =========================================================
		-- Test Point3 comparison
		assert_point3_equal [0,0,0] [0,0,0]
		assert_not_equal [0,0,0] [4,0,0]
		assert_not_equal [0,0,0] [0,5,0]
		assert_not_equal [0,0,0] [0,0,6] 
		assert_point3_equal [4.8,0,0] [4.9,0,0] tolerance: 0.2 -- This should pass
		assert_not_equal [4.8,0,0] [4.9,0,0] 
		
		-- =========================================================
		-- Test matrix comparison
		local mat1 = (matrixFromNormal [15,15,60])
		assert_matrix_equal (matrixFromNormal [15,15,60]) mat1
		assert_not_equal (matrixFromNormal [15,15,55]) mat1

		-- =========================================================
		-- Test assert_eulerAngles_equal comparison
		assert_eulerAngles_equal (eulerAngles 0 90 0) (eulerAngles 0 90 0)
		assert_eulerAngles_equal (((eulerAngles 90 10 90) as quat) as eulerangles) (eulerAngles 90 10 90)

		-- =========================================================
		-- Test assert_quat_equal comparison
		assert_quat_equal (quat 0.707107 0 0 0.707107) (quat 0.707107 0 0 0.707107)
		-- ((quat 0.707107 0 0 -0.707107) as matrix3) as quat  --> (quat -0.707107 0 0 0.707107)
		assert_quat_equal (((quat 0.707107 0 0 -0.707107) as matrix3) as quat) (quat 0.707107 0 0 -0.707107)
		-- =========================================================
		-- Test array comparison
		assert_array_equal #(1) #(1)
		assert_array_equal #(1.) #(1.)
		assert_array_equal #([1,2]) #([1,2])
		assert_array_equal #([1,2,3]) #([1,2,3])
		assert_array_equal #([1,2,3,4]) #([1,2,3,4])
		assert_array_equal #(matrix3 1) #(matrix3 1)
		assert_array_equal #("aa") #("aa")
		assert_array_equal #(1.) #(1.1) tolerance:0.11
		assert_array_equal #("aa") #("AA")
		assert_array_equal #(transmatrix [1.0009,1,1]) #(transmatrix [1,1,1])
		
		-- =========================================================
		-- Test bitarray comparison
		assert_bitarray_equal #{} #{}
		assert_bitarray_equal #{1..5,10} #{1..5,10}
		local ba = #{1..5,10}
		ba[10] = false
		assert_bitarray_equal #{1..5} ba

		-- =========================================================
		-- Test assert_matchpattern
		local s="text1"
		assert_matchpattern "text?" s -- returns true
		assert_matchpattern "T*" s -- returns true
		assert_matchpattern "*T*" s -- returns true

		-- =========================================================
		AssertReporter.LogMessage "End of the tests"
		print (AssertReporter.GetMessages())
		assert ((AssertReporter.GetAssertFailures()).count == 0) -- use the old style assert.
		print "All done..."
		ok
	),
	
	Tests = 
	#(
		testAsserts
	)
)

if undefined == run_test_fixture then
(
	format "-- Unit test fixture not found. Running tests outside of fixture.\n"
	TestCase = MxsDiagnosticAssertTests()
	TestCase.Setup()
	TestCase.testAsserts()
	TestCase.Teardown()
)
else
(
	format "-- Fixture found\n"
	run_test_fixture MxsDiagnosticAssertTests script:(getThisScriptFilename())
)