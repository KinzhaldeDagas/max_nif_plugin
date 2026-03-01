/*
    @param TestName:    Morph.UnitTest
    @param Author:      Larry Minton
    @param Created:     May 18, 2022
    @param LastMod:     
    @param ModBy:       
    @param Desc:        Test Morph
    @param Status:      Online
*/

fileIn "MxsUnit/MxsUnit.ms"

struct Morph_Tests
(

	fn TestShutdownLeakage = 
	(
		print "running TestShutdownLeakage"
		local b=box()
		b.baseobject = createinstance morph forceCreate:true
		delete b
	),
	
	Tests = 
	#(
		TestShutdownLeakage
	)
)

run_test_fixture Morph_Tests script:(getThisScriptFilename())
