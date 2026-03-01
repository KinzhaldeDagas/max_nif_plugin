/*
    @param TestName:        LoadDllsFromDir.UnitTest
    @param Author:          Attila Szabo
    @param Created:         Aug.23.2010
    @param LastMod:         September 16, 2021
    @param ModBy:           Nhien Ton
    @param Description:     Unit tests for the loadDllsFromDir and the PluginManager interface
    @param Status:          Online
*/

fileIn "MxsUnit/MxsUnit.ms"
fileIn "PluginDllUtilities.mxs"

struct UnitTest_LoadDllsFromDir
(	
private
	originalPluginDllCount,
	
	-- loadedStateOption = true measn use a loaded plug-in for testing
	-- loadedStateOption = false means use a deferred plug-in for testing
	function doTestLoadDllsFromDir loadedStateOption forceLoadOption =
	(
		-- find first plug-in dll that has the specified loaded state (loaded or deferred)
		plugins = getPluginDllsFullPath loaded:loadedStateOption
		if (plugins.count > 0) then
		(
			with quiet true 
			(
				local pluginDllPath = getFilenamePath plugins[1]
				local pluginDllFileName = filenameFromPath plugins[1]
				loadDllsFromDir pluginDllPath pluginDllFileName forceLoad:forceLoadOption
			)
			local actualLoadedState = isPluginDllLoaded plugins[1]
			if (loadedStateOption) then
			(
				-- a loaded plug-in's state should be loaded whether it was force loaded or not
				assert_true actualLoadedState
			)
			else
			(
				if (false == forceLoadOption) then 
				(
					-- a deferred plug-in's state should be deferred if it was defer loaded
					assert_false actualLoadedState
				)
				else
				(
					-- a deferred plug-in's state should be loaded if it was force loaded
					assert_true actualLoadedState
				)
			)
		)

		assert ((countPluginDlls()) == originalPluginDllCount)
	),
	
public
	function setup =
	(
		resetMaxFile #noprompt
		originalPluginDllCount =  countPluginDlls()
	),		
	
	function testDeferLoadingLoadedPluginDlls = 
	(
		local loadedState = true
		local forceLoad = false
		doTestLoadDllsFromDir loadedState forceLoad
	),

	function testForceLoadingLoadedPluginDlls = 
	(
		local loadedState = true
		local forceLoad = true
		doTestLoadDllsFromDir loadedState forceLoad
	),

	function testDeferLoadingDeferredPluginDlls = 
	(
		local loadedState = false
		local forceLoad = false
		doTestLoadDllsFromDir loadedState forceLoad
	),

	function testForceLoadingDeferredPluginDlls = 
	(
		local loadedState = false
		local forceLoad = true
		doTestLoadDllsFromDir loadedState forceLoad
	),
	
	function testPluginManager =
	(
		format "Looking for pluginmanager.gup\n"
		indexPluginManager = -1
		for i = 1 to pluginManager.pluginDllCount do 
		(
			if ((pluginManager.pluginDllName i) == "pluginmanager.gup") then 
			(
				format "Found pluginmanager.gup with index %\n" i
				indexPluginManager = i
				exit
			)		
		)

		assert_not_equal -1 indexPluginManager message:"Didn't find the pluginmanager.gup"
		if indexPluginManager != -1 then 
		(
			format "Testing the pluginmanager interface\n"
			maxRootDir = GetDir #maxroot
			stdPlugsDir = pathConfig.appendPath maxRootDir "stdplugs\\"
			pluginManagerPath = pathConfig.appendPath stdPlugsDir "pluginmanager.gup"
			assert_string_equal "pluginmanager.gup" (pluginManager.pluginDllName indexPluginManager) message:"pluginDllName didn't return expected name"
			assert_string_equal stdPlugsDir (pluginManager.pluginDllDirectory indexPluginManager) message:"pluginDllDirectory didn't return expected directory"
			assert_string_equal pluginManagerPath (pluginManager.pluginDllFullPath indexPluginManager) message:"pluginDllFullPath didn't return expected path"
		)
	),

	Tests = 
	#(
		testDeferLoadingLoadedPluginDlls,
		testForceLoadingLoadedPluginDlls,
		testDeferLoadingDeferredPluginDlls,
		testForceLoadingDeferredPluginDlls,
		testPluginManager
	)
)

-- uncomment these lines to run tests outside of the mxs unit test framework, which "eats" exceptions making it harder to debug your tests
--test = UnitTest_LoadDllsFromDir()
--test.testDeferringLoadedPluginDlls()

-- When in secure mode, none on these tests apply since loadDllsFromDir is not available to be used
if (not maxops.isInSecureMode()) then
(
	run_test_fixture UnitTest_LoadDllsFromDir script:(getThisScriptFilename())	
)
else
(
	format "loadDllsFromDir is not available in secure mode, therefore no tests have been executed.\n"
)

