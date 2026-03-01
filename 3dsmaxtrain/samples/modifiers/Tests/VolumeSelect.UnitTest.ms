
fileIn ((getFileNamePath(getSourceFileName())) + "/ValidateObjectBoundsAtTime.mxs")
--------------------------------------------------------------------------------------
-- Tests for Volume Select Modifier
--
-- Author: Mathew Kaustinen
-- Created: July 2023
--------------------------------------------------------------------------------------

Struct VolumeSelectUnitTests 
( 
	Function setup = 
	( 
	),

	Function teardown = 
	( 
	),

		
	Function Mesh_Test = 
	(
		testName = "Volume_Select_Mesh_Test"
		format "%\n" testName
		filePath = getFileNamePath(getSourceFileName())
		fileName = filePath + "/TestData/" + testName + ".max"
		isLoaded = loadMaxFile fileName quiet:true useFileUnits:true
		Assert_true isLoaded message: ("Error " + testName + " (load)")
		if (isLoaded) do
		(
			select $cthulu_Select

			-- Make sure Turn to Poly is off
			$.modifiers[3].enabled = false
			
			-- Test Verts
			$.modifiers[#Vol__Select].level = 1

			-- Check bounds at some key times
			$.modifiers[#Vol__Select].UseAffectRegion = off
			ValidateObjectBoundsAtTime $ 0f #([-776.87,362.811,-6.00714], [-456.167,796.47,486.374]) (testName + " Bounds error1")	
			ValidateObjectBoundsAtTime $ 40f #([-776.87,352.839,-6.00714], [-446.232,796.47,486.374]) (testName + " Bounds error2")
			ValidateObjectBoundsAtTime $ 50f #([-785.918,352.839,-12.5061], [-446.232,796.47,496.299]) (testName + " Bounds error3")
			ValidateObjectBoundsAtTime $ 65f #([-786.86,362.811,-6.00714] , [-456.167,796.47,486.374]) (testName + " Bounds error4")	
			
			-- Try again with soft select enabled
			$.modifiers[#Vol__Select].UseAffectRegion = on
			ValidateObjectBoundsAtTime $ 0f #([-776.87,362.811,-6.00714], [-456.167,796.47,486.374]) (testName + " Bounds error5")	
			ValidateObjectBoundsAtTime $ 40f #([-776.87,352.839,-7.80851], [-446.232,796.47,486.374]) (testName + " Bounds error6")
			ValidateObjectBoundsAtTime $ 50f #([-786.626,352.839,-13.6558], [-446.232,798.178,496.299]) (testName + " Bounds error7")
			ValidateObjectBoundsAtTime $ 65f #([-786.86,358.923,-6.00714], [-456.167,796.47,486.374]) (testName + " Bounds error8")	
			
			-- Now faces
			$.modifiers[#Vol__Select].level = 2
			$.modifiers[#Vol__Select].UseAffectRegion = off
			ValidateObjectBoundsAtTime $ 0f #([-776.87,362.811,-6.00714], [-456.167,796.47,486.374]) (testName + " Bounds error9")	
			ValidateObjectBoundsAtTime $ 40f #([-776.87,352.839,-6.00714], [-446.232,796.47,486.374]) (testName + " Bounds error10")
			ValidateObjectBoundsAtTime $ 50f #([-785.191,352.839,-11.6435], [-446.232,796.47,496.299]) (testName + " Bounds error11")
			ValidateObjectBoundsAtTime $ 65f #([-786.86,362.811,-6.00714] , [-456.167,796.47,486.374]) (testName + " Bounds error12")
			
			-- Now do it again for poly
			$.modifiers[3].enabled = true
			
			-- Test Verts
			$.modifiers[#Vol__Select].level = 1

			-- Check bounds at some key times
			$.modifiers[#Vol__Select].UseAffectRegion = off
			ValidateObjectBoundsAtTime $ 0f #([-776.87,362.811,-6.00714], [-456.167,796.47,486.374]) (testName + " Bounds error13")	
			ValidateObjectBoundsAtTime $ 40f #([-776.87,352.839,-6.00714], [-446.232,796.47,486.374]) (testName + " Bounds error14")
			ValidateObjectBoundsAtTime $ 50f #([-785.923,352.839,-12.5251], [-446.232,796.47,496.344]) (testName + " Bounds error15")
			ValidateObjectBoundsAtTime $ 65f #([-786.86,362.811,-6.00714] , [-456.167,796.47,486.374]) (testName + " Bounds error16")	
			
			-- Try again with soft select enabled
			$.modifiers[#Vol__Select].UseAffectRegion = on
			ValidateObjectBoundsAtTime $ 0f #([-776.87,362.811,-6.00714], [-456.167,796.47,486.374]) (testName + " Bounds error17")	
			ValidateObjectBoundsAtTime $ 40f #([-776.87,352.839,-7.81482], [-446.232,796.47,486.374]) (testName + " Bounds error18")
			ValidateObjectBoundsAtTime $ 50f #([-786.626,352.839,-13.6558], [-446.232,798.178,496.344]) (testName + " Bounds error19")
			ValidateObjectBoundsAtTime $ 65f #([-786.86,358.923,-6.00714], [-456.167,796.47,486.374]) (testName + " Bounds error20")	
			
			-- Now faces
			$.modifiers[#Vol__Select].level = 2
			$.modifiers[#Vol__Select].UseAffectRegion = off
			ValidateObjectBoundsAtTime $ 0f #([-776.87,362.811,-6.00714], [-456.167,796.47,486.374]) (testName + " Bounds error21")	
			ValidateObjectBoundsAtTime $ 40f #([-776.87,352.839,-6.00714], [-446.232,796.47,486.374]) (testName + " Bounds error22")
			ValidateObjectBoundsAtTime $ 50f #([-785.923,352.839,-12.5251], [-446.232,796.47,496.344]) (testName + " Bounds error23")
			ValidateObjectBoundsAtTime $ 65f #([-786.86,362.811,-6.00714] , [-456.167,796.47,486.374]) (testName + " Bounds error24")
		)
	),
	
	Function Texture_Test = 
	(
		testName = "Volume_Select_Texture_Test"
		format "%\n" testName
		filePath = getFileNamePath(getSourceFileName())
		fileName = filePath + "/TestData/" + testName + ".max"
		isLoaded = loadMaxFile fileName quiet:true useFileUnits:true
		Assert_true isLoaded message: ("Error " + testName + " (load)")
		if (isLoaded) do
		(
			select $Plane001
			
			-- Make sure Turn to Poly is off
			$.modifiers[4].enabled = false
			
			-- Check bounds
			ValidateObjectBoundsAtTime $ 0f #([-131.433,-138.119,0], [133.005,129.13,109.352]) (testName + " Bounds error1")
			
			-- Check again as poly
			$.modifiers[4].enabled = true
			
			-- Check bounds
			ValidateObjectBoundsAtTime $ 0f #([-131.203,-138.149,0], [133.289,129.11,109.277]) (testName + " Bounds error2")
		)
	),
	
		
	Function Vertex_Color_Test = 
	(
		testName = "Volume_Select_Vertex_Color_Test"
		format "%\n" testName
		filePath = getFileNamePath(getSourceFileName())
		fileName = filePath + "/TestData/" + testName + ".max"
		isLoaded = loadMaxFile fileName quiet:true useFileUnits:true
		Assert_true isLoaded message: ("Error " + testName + " (load)")
		if (isLoaded) do
		(
			select $GeoSphere001
			
			-- Make sure Turn to Poly is off
			$.modifiers[3].enabled = false
			-- Vertex Selection
			$.modifiers[#Vol__Select].level = 1
			
			-- Check bounds
			ValidateObjectBoundsAtTime $ 0f #([-47.1232,-40.0472,-38.3677], [37.6312,37.0789,48.6555]) (testName + " Bounds error1")
			
			-- Face Selection
			$.modifiers[#Vol__Select].level = 2

			-- Check bounds
			ValidateObjectBoundsAtTime $ 0f #([-53.319,-47.1463,-38.3677], [37.6312,37.0789,51.8159]) (testName + " Bounds error2")			
			
			-- Check again as poly
			$.modifiers[3].enabled = true
			
			-- Vertex Selection
			$.modifiers[#Vol__Select].level = 1
			
			-- Check bounds
			ValidateObjectBoundsAtTime $ 0f #([-47.1232,-40.0472,-38.3677], [37.6312,37.0789,48.6555]) (testName + " Bounds error3")
			
			-- Face Selection
			$.modifiers[#Vol__Select].level = 2

			-- Check bounds
			ValidateObjectBoundsAtTime $ 0f #([-53.9041,-50.1379,-38.3677], [37.6312,37.0789,52.6614]) (testName + " Bounds error4")	
		)
	),
	
	Function Gizmo_Test = 
	(
		testName = "Volume_Select_Gizmo_Test"
		format "%\n" testName
		filePath = getFileNamePath(getSourceFileName())
		fileName = filePath + "/TestData/" + testName + ".max"
		isLoaded = loadMaxFile fileName quiet:true useFileUnits:true
		Assert_true isLoaded message: ("Error " + testName + " (load)")
		if (isLoaded) do
		(
			select $GeoSphere001
			
			-- Make sure Turn to Poly is off
			$.modifiers[3].enabled = false		
			
			-- Check bounds for Box Mode hard selection
			$.modifiers[#Vol__Select].level = 1
			$.modifiers[#Vol__Select].volume = 0
			$.modifiers[#Vol__Select].UseAffectRegion = off
			ValidateObjectBoundsAtTime $ 0f #([-49.6041,-39.6564,-48.8677], [37.6312,47.5789,48.8677]) (testName + " Bounds error1")
			ValidateObjectBoundsAtTime $ 100f #([-39.1041,-50.1564,-48.8677], [48.1312,37.0789,48.8677] ) (testName + " Bounds error2")
			
			-- and again for soft select
			$.modifiers[#Vol__Select].UseAffectRegion = on
			ValidateObjectBoundsAtTime $ 0f #([-49.6041,-40.2012,-48.8677], [37.6312,47.5789,48.8677]) (testName + " Bounds error3")
			ValidateObjectBoundsAtTime $ 100f #([-39.1041,-50.1564,-48.8677], [48.1312,40.1112,48.8677] ) (testName + " Bounds error4")
			
			-- repeat for cylinder
			$.modifiers[#Vol__Select].volume = 1
			$.modifiers[#Vol__Select].UseAffectRegion = off
			ValidateObjectBoundsAtTime $ 0f #([-49.6041,-39.6564,-41.6675], [37.6312,47.3111,41.5953]) (testName + " Bounds error5")
			ValidateObjectBoundsAtTime $ 100f #([-39.1041,-50.0894,-43.8113], [48.1312,37.0789,43.8113]) (testName + " Bounds error6")
		
			-- and again for soft select
			$.modifiers[#Vol__Select].UseAffectRegion = on
			ValidateObjectBoundsAtTime $ 0f #([-49.6041,-39.6564,-43.7211], [37.6312,47.417,43.7131]) (testName + " Bounds error7")
			ValidateObjectBoundsAtTime $ 100f #([-39.1041,-50.0894,-45.5311], [48.1312,37.2794,45.5364]) (testName + " Bounds error8")
			
			-- and Sphere
			$.modifiers[#Vol__Select].volume = 2
			$.modifiers[#Vol__Select].UseAffectRegion = off
			ValidateObjectBoundsAtTime $ 0f #([-49.6041,-39.6564,-48.8158], [37.6312,47.3111,48.8158]) (testName + " Bounds error9")
			ValidateObjectBoundsAtTime $ 100f #([-39.1041,-50.0894,-48.8677], [48.1312,37.0789,48.8677]) (testName + " Bounds error10")
			
			-- and again for soft select
			$.modifiers[#Vol__Select].UseAffectRegion = on
			ValidateObjectBoundsAtTime $ 0f #([-49.6041,-39.6564,-48.8158], [37.6312,47.417,48.8158]) (testName + " Bounds error11")
			ValidateObjectBoundsAtTime $ 100f #([-39.1041,-50.0894,-48.8677], [48.1312,37.2794,48.8677]) (testName + " Bounds error12")

			-- check faces for good measure
			$.modifiers[#Vol__Select].level = 2
			ValidateObjectBoundsAtTime $ 0f #([-49.6041,-39.6564,-48.7313], [37.6312,46.9772,48.6601]) (testName + " Bounds error13")
			ValidateObjectBoundsAtTime $ 100f #([-39.1041,-49.8886,-48.8677], [48.1312,37.0789,48.8677]) (testName + " Bounds error14")	
			
			-- Turn to poly and do it all over again
			$.modifiers[3].enabled = true		
			
			-- Check bounds for Box Mode hard selection
			$.modifiers[#Vol__Select].level = 1
			$.modifiers[#Vol__Select].volume = 0
			$.modifiers[#Vol__Select].UseAffectRegion = off
			ValidateObjectBoundsAtTime $ 0f #([-49.6041,-39.6564,-48.8677], [37.6312,47.5789,48.8677]) (testName + " Bounds error15")
			ValidateObjectBoundsAtTime $ 100f #([-39.1041,-50.1564,-48.8677], [48.1312,37.0789,48.8677] ) (testName + " Bounds error16")
			
			-- and again for soft select
			$.modifiers[#Vol__Select].UseAffectRegion = on
			ValidateObjectBoundsAtTime $ 0f #([-49.6041,-40.2012,-48.8677], [37.6312,47.5789,48.8677]) (testName + " Bounds error17")
			ValidateObjectBoundsAtTime $ 100f #([-39.1041,-50.1564,-48.8677], [48.1312,40.1112,48.8677] ) (testName + " Bounds error18")
			
			-- repeat for cylinder
			$.modifiers[#Vol__Select].volume = 1
			$.modifiers[#Vol__Select].UseAffectRegion = off
			ValidateObjectBoundsAtTime $ 0f #([-49.6041,-39.6564,-41.6675], [37.6312,47.3111,41.5953]) (testName + " Bounds error19")
			ValidateObjectBoundsAtTime $ 100f #([-39.1041,-50.0894,-43.8113], [48.1312,37.0789,43.8113]) (testName + " Bounds error20")
		
			-- and again for soft select
			$.modifiers[#Vol__Select].UseAffectRegion = on
			ValidateObjectBoundsAtTime $ 0f #([-49.6041,-39.6564,-43.7211], [37.6312,47.417,43.7131]) (testName + " Bounds error21")
			ValidateObjectBoundsAtTime $ 100f #([-39.1041,-50.0894,-45.5311], [48.1312,37.2794,45.5364]) (testName + " Bounds error22")
			
			-- and Sphere
			$.modifiers[#Vol__Select].volume = 2
			$.modifiers[#Vol__Select].UseAffectRegion = off
			ValidateObjectBoundsAtTime $ 0f #([-49.6041,-39.6564,-48.8158], [37.6312,47.3111,48.8158]) (testName + " Bounds error23")
			ValidateObjectBoundsAtTime $ 100f #([-39.1041,-50.0894,-48.8677], [48.1312,37.0789,48.8677]) (testName + " Bounds error24")
			
			-- and again for soft select
			$.modifiers[#Vol__Select].UseAffectRegion = on
			ValidateObjectBoundsAtTime $ 0f #([-49.6041,-39.6564,-48.8158], [37.6312,47.417,48.8158]) (testName + " Bounds error25")
			ValidateObjectBoundsAtTime $ 100f #([-39.1041,-50.0894,-48.8677], [48.1312,37.2794,48.8677]) (testName + " Bounds error26")

			-- check faces for good measure
			$.modifiers[#Vol__Select].level = 2
			ValidateObjectBoundsAtTime $ 0f #([-49.6041,-39.6564,-48.8158], [37.6312,47.3111,48.8158]) (testName + " Bounds error27")
			ValidateObjectBoundsAtTime $ 100f #([-39.1041,-50.0894,-48.8677], [48.1312,37.0789,48.8677]) (testName + " Bounds error28")	
		)
	),
	
	Function Smooth_Group_Test = 
	(
		testName = "Volume_Select_Smoothing_Group_Test"
		format "%\n" testName
		filePath = getFileNamePath(getSourceFileName())
		fileName = filePath + "/TestData/" + testName + ".max"
		isLoaded = loadMaxFile fileName quiet:true useFileUnits:true
		Assert_true isLoaded message: ("Error " + testName + " (load)")
		if (isLoaded) do
		(
			select $GeoSphere001
			
			-- Vertex Selection
			$.modifiers[#Vol__Select].level = 1

			-- Make sure Turn to Poly is off
			$.modifiers[3].enabled = false
			
			-- Check bounds for Smoothing Group 1
			$.modifiers[#Vol__Select].smGroup = 1
			ValidateObjectBoundsAtTime $ 0f #([-49.6041,-50.1564,-48.8677], [48.1312,47.5789,48.8677]) (testName + " Bounds error1")
			
			-- Check bounds for Smoothing Group 2
			$.modifiers[#Vol__Select].smGroup = 2
			ValidateObjectBoundsAtTime $ 0f #([-39.1041,-49.7068,-48.3198], [40.5009,47.1293,48.4009]) (testName + " Bounds error2")
			
			-- Check bounds for Smoothing Group 3 (empty)
			$.modifiers[#Vol__Select].smGroup = 3
			ValidateObjectBoundsAtTime $ 0f #([-39.1041,-39.6564,-38.3677], [37.6312,37.0789,38.3677]) (testName + " Bounds error3")
			
			-- Again for Poly 
			$.modifiers[3].enabled = true
			
			-- Check bounds for Smoothing Group 1
			$.modifiers[#Vol__Select].smGroup = 1
			ValidateObjectBoundsAtTime $ 0f #([-49.6041,-50.1564,-48.8677], [48.1312,47.5789,48.8677]) (testName + " Bounds error4")
			
			-- Check bounds for Smoothing Group 2
			$.modifiers[#Vol__Select].smGroup = 2
			ValidateObjectBoundsAtTime $ 0f #([-39.1041,-49.7068,-48.3198], [40.5009,47.1293,48.4009]) (testName + " Bounds error5")
			
			-- Check bounds for Smoothing Group 3 (empty)
			$.modifiers[#Vol__Select].smGroup = 3
			ValidateObjectBoundsAtTime $ 0f #([-39.1041,-39.6564,-38.3677], [37.6312,37.0789,38.3677]) (testName + " Bounds error6")
			
			-- Now for Faces 
			$.modifiers[#Vol__Select].level = 2

			-- Make sure Turn to Poly if off
			$.modifiers[3].enabled = false
			
			-- Check bounds for Smoothing Group 1
			$.modifiers[#Vol__Select].smGroup = 1
			ValidateObjectBoundsAtTime $ 0f #([-49.6041,-50.1564,-48.8677], [48.1312,47.5789,48.8677]) (testName + " Bounds error7")
			
			-- Check bounds for Smoothing Group 2
			$.modifiers[#Vol__Select].smGroup = 2
			ValidateObjectBoundsAtTime $ 0f #([-39.1041,-49.7068,-48.3198], [40.5009,47.1293,48.4009]) (testName + " Bounds error8")
			
			-- Check bounds for Smoothing Group 3 (empty)
			$.modifiers[#Vol__Select].smGroup = 3
			ValidateObjectBoundsAtTime $ 0f #([-39.1041,-39.6564,-38.3677], [37.6312,37.0789,38.3677]) (testName + " Bounds error9")
			
			-- Again for Poly 
			$.modifiers[3].enabled = true
			
			-- Check bounds for Smoothing Group 1
			$.modifiers[#Vol__Select].smGroup = 1
			ValidateObjectBoundsAtTime $ 0f #([-49.6041,-50.1564,-48.8677], [48.1312,47.5789,48.8677]) (testName + " Bounds error10")
			
			-- Check bounds for Smoothing Group 2
			$.modifiers[#Vol__Select].smGroup = 2
			ValidateObjectBoundsAtTime $ 0f #([-39.1041,-49.9731,-48.5425], [41.7972,47.3956,48.6601]) (testName + " Bounds error11")
			
			-- Check bounds for Smoothing Group 3 (empty)
			$.modifiers[#Vol__Select].smGroup = 3
			ValidateObjectBoundsAtTime $ 0f #([-39.1041,-39.6564,-38.3677], [37.6312,37.0789,38.3677]) (testName + " Bounds error12")			
		)
	),

	Function MatID_Test = 
	(
		testName = "Volume_Select_Material_ID_Test"
		format "%\n" testName
		filePath = getFileNamePath(getSourceFileName())
		fileName = filePath + "/TestData/" + testName + ".max"
		isLoaded = loadMaxFile fileName quiet:true useFileUnits:true
		Assert_true isLoaded message: ("Error " + testName + " (load)")
		if (isLoaded) do
		(
			select $GeoSphere001
			
			-- Vertex Selection
			$.modifiers[#Vol__Select].level = 1

			-- Make sure Turn to Poly if off
			$.modifiers[3].enabled = false
			
			-- Check bounds for Mat ID 1
			$.modifiers[#Vol__Select].matID = 1
			ValidateObjectBoundsAtTime $ 0f #([-39.1041,-46.406,-45.3386], [38.986,43.8285,45.2329]) (testName + " Bounds error1")
			
			-- Check bounds for Mat ID 2
			$.modifiers[#Vol__Select].matID = 2
			ValidateObjectBoundsAtTime $ 0f #([-49.6041,-50.1564,-48.8677], [48.1312,47.5789,48.8677]) (testName + " Bounds error2")
			
			-- Check bounds for Mat ID 3 (empty)
			$.modifiers[#Vol__Select].matID = 3
			ValidateObjectBoundsAtTime $ 0f #([-39.1041,-39.6564,-38.3677], [37.6312,37.0789,38.3677]) (testName + " Bounds error3")

			-- Test Again for Poly
			$.modifiers[3].enabled = true
			
			$.modifiers[#Vol__Select].matID = 1
			ValidateObjectBoundsAtTime $ 0f #([-39.1041,-46.406,-45.3386], [38.986,43.8285,45.2329]) (testName + " Bounds error4")
			
			-- Check bounds for Mat ID 2
			$.modifiers[#Vol__Select].matID = 2
			ValidateObjectBoundsAtTime $ 0f #([-49.6041,-50.1564,-48.8677], [48.1312,47.5789,48.8677]) (testName + " Bounds error5")
			
			-- Check bounds for Mat ID 3 (empty)
			$.modifiers[#Vol__Select].matID = 3
			ValidateObjectBoundsAtTime $ 0f #([-39.1041,-39.6564,-38.3677], [37.6312,37.0789,38.3677]) (testName + " Bounds error6")			
			
			-- And Again for Faces
			$.modifiers[#Vol__Select].level = 2

			-- Make sure Turn to Poly is off
			$.modifiers[3].enabled = false
			
			-- Check bounds for Mat ID 1
			$.modifiers[#Vol__Select].matID = 1
			ValidateObjectBoundsAtTime $ 0f #([-39.1041,-46.406,-45.3386], [38.986,43.8285,45.2329]) (testName + " Bounds error7")
			
			-- Check bounds for Mat ID 2
			$.modifiers[#Vol__Select].matID = 2
			ValidateObjectBoundsAtTime $ 0f #([-49.6041,-50.1564,-48.8677], [48.1312,47.5789,48.8677]) (testName + " Bounds error8")
			
			-- Check bounds for Mat ID 3 (empty)
			$.modifiers[#Vol__Select].matID = 3
			ValidateObjectBoundsAtTime $ 0f #([-39.1041,-39.6564,-38.3677], [37.6312,37.0789,38.3677]) (testName + " Bounds error9")

			-- Test Again for Poly
			$.modifiers[3].enabled = true
			
			$.modifiers[#Vol__Select].matID = 1
			ValidateObjectBoundsAtTime $ 0f #([-39.1041,-47.316,-46.0119], [40.4354,44.7385,46.0403]) (testName + " Bounds error10")
			
			-- Check bounds for Mat ID 2
			$.modifiers[#Vol__Select].matID = 2
			ValidateObjectBoundsAtTime $ 0f #([-49.6041,-50.1564,-48.8677], [48.1312,47.5789,48.8677]) (testName + " Bounds error11")
			
			-- Check bounds for Mat ID 3 (empty)
			$.modifiers[#Vol__Select].matID = 3
			ValidateObjectBoundsAtTime $ 0f #([-39.1041,-39.6564,-38.3677], [37.6312,37.0789,38.3677]) (testName + " Bounds error12")	
		)
	),
	
	-- MatID and Smoothing Group Selection are Animatable in 2024.3 onwards
	-- confirm this works
	Function Animatable_Test = 
	(
		testName = "Volume_Select_Animated_Test"
		format "%\n" testName
		filePath = getFileNamePath(getSourceFileName())
		fileName = filePath + "/TestData/" + testName + ".max"
		isLoaded = loadMaxFile fileName quiet:true useFileUnits:true
		Assert_true isLoaded message: ("Error " + testName + " (load)")
		if (isLoaded) do
		(
			-- Smoothing Group Test
			select $GeoSphere_SMG
			-- Make sure Turn to Poly is off
			$.modifiers[3].enabled = false
			ValidateObjectBoundsAtTime $ 0f #([-126.954,-50.1564,-48.8677], [-29.2188,47.5789,48.8677]) (testName + " Bounds error1")
			ValidateObjectBoundsAtTime $ 100f #([-116.454,-49.7068,-48.3198], [-36.8491,47.1293,48.4009]) (testName + " Bounds error2")
			
			-- Again for Poly
			$.modifiers[3].enabled = true
			ValidateObjectBoundsAtTime $ 0f #([-126.954,-50.1564,-48.8677], [-29.2188,47.5789,48.8677]) (testName + " Bounds error3")
			ValidateObjectBoundsAtTime $ 100f #([-116.454,-49.7068,-48.3198], [-36.8491,47.1293,48.4009]) (testName + " Bounds error4")
			
			-- Mat ID Test
			select $GeoSphere_MatID
			-- Make sure Turn to Poly is off
			$.modifiers[3].enabled = false					
			ValidateObjectBoundsAtTime $ 0f #([52.5762,-46.406,-45.3386], [130.666,43.8285,45.2329]) (testName + " Bounds error5")
			ValidateObjectBoundsAtTime $ 100f #([42.0762,-50.1564,-48.8677], [139.812,47.5789,48.8677]) (testName + " Bounds error6")
			
			-- Again for Poly
			$.modifiers[3].enabled = true					
			ValidateObjectBoundsAtTime $ 0f #([52.5762,-47.316,-46.0119], [132.116,44.7385,46.0403]) (testName + " Bounds error7")
			ValidateObjectBoundsAtTime $ 100f #([42.0762,-50.1564,-48.8677], [139.812,47.5789,48.8677]) (testName + " Bounds error8")
		)
	),
	
	function VertDistCount threshold = 
	(
		objClass = classof $
		vertPos = #()

		if ((objClass == Editable_Poly) or (objClass == PolyMeshObject)) then
		(
			numVerts = polyop.getNumVerts $
			--vertes position
			vertPos.count = numVerts
			for index = 1 to numVerts do
			(
				vertPos[index] = polyop.getVert $ index
			)

		)
		if (objClass == Editable_Mesh) then
		(
			numVerts = meshop.getNumVerts $
			--vertes position
			vertPos.count = numVerts
			for index = 1 to numVerts do
			(
				vertPos[index] = meshop.getVert $ index
			)

		)

			--output code
		ct = 0
		for index = 1 to numVerts do
		(
			d = Length vertPos[index]
			if (d > threshold) then
			(
				ct = ct + 1
			)
		)
		
		return ct
	),	
	Function StackSelection_Test = 
	(
		(
			testName = "Volume_Select_StackSelection_Test"
			format "%\n" testName
			format "Volume Select Test\n"
			filePath = getFileNamePath(getSourceFileName())
			fileName = filePath + "/TestData/" + testName + ".max"
			isLoaded = loadMaxFile fileName quiet:true useFileUnits:true
			Assert_true isLoaded message: ("Error " + testName + " (load)")
			if (isLoaded) do
			(
				-- Smoothing Group Test
				select $Sphere001
				modPanel.addModToSelection (Vol__Select ()) ui:on
				$.modifiers[#Vol__Select].level = 1	
				$.modifiers[#Vol__Select].volume = 7
				$.modifiers[#Vol__Select].autofit = off
				$.modifiers[#Vol__Select].UseAffectRegion = on
				modPanel.addModToSelection (Push ()) ui:on
				$.modifiers[#Push].Push_Value = 5				
				threshold = 23.49195
				sliderTime = 0f
				ct = VertDistCount threshold
				format "Number moved verts %\n" ct
				Assert_true (ct == 0)
				sliderTime = 15f
				ct = VertDistCount threshold
				format "Number moved verts %\n" ct
				Assert_true (ct == 482)
				sliderTime = 25f
				ct = VertDistCount threshold 			 
				format "Number moved verts %\n" ct
				Assert_true (ct == 229)
			)
		)
		(
			testName = "Volume_Select_StackSelection_Test"
			format "%\n" testName
			format "Poly Select Test\n"
			filePath = getFileNamePath(getSourceFileName())
			fileName = filePath + "/TestData/" + testName + ".max"
			isLoaded = loadMaxFile fileName quiet:true useFileUnits:true
			Assert_true isLoaded message: ("Error " + testName + " (load)")
			if (isLoaded) do
			(
				-- Smoothing Group Test
				select $Sphere001
				modPanel.addModToSelection (Vol__Select ()) ui:on
				modPanel.addModToSelection (Poly_Select ()) ui:on
				subobjectLevel = 1
				$.modifiers[#Poly_Select].useStackSelection = on
				$.modifiers[#Poly_Select].useSoftSelection = on
				modPanel.addModToSelection (Push ()) ui:on
				$.modifiers[#Push].Push_Value = 5				
				threshold = 23.49195
				sliderTime = 0f
				ct = VertDistCount threshold
				format "Number moved verts %\n" ct
				Assert_true (ct == 0)
				sliderTime = 15f
				ct = VertDistCount threshold
				format "Number moved verts %\n" ct
				Assert_true (ct == 482)
				sliderTime = 25f
				ct = VertDistCount threshold 			 
				format "Number moved verts %\n" ct
				Assert_true (ct == 229)
			)
		)
		(
			testName = "Volume_Select_StackSelection_Test"
			format "%\n" testName
			format "Mesh Select Test\n"
			filePath = getFileNamePath(getSourceFileName())
			fileName = filePath + "/TestData/" + testName + ".max"
			isLoaded = loadMaxFile fileName quiet:true useFileUnits:true
			Assert_true isLoaded message: ("Error " + testName + " (load)")
			if (isLoaded) do
			(
				-- Smoothing Group Test
				select $Sphere001
				modPanel.addModToSelection (Vol__Select ()) ui:on
				modPanel.addModToSelection (Mesh_Select ()) ui:on
				subobjectLevel = 1
				$.modifiers[#Mesh_Select].useStackSelection = on
				$.modifiers[#Mesh_Select].useSoftSelection = on
				modPanel.addModToSelection (Push ()) ui:on
				$.modifiers[#Push].Push_Value = 5				
				threshold = 23.49195
				sliderTime = 0f
				ct = VertDistCount threshold
				format "Number moved verts %\n" ct
				Assert_true (ct == 0)
				sliderTime = 15f
				ct = VertDistCount threshold
				format "Number moved verts %\n" ct
				Assert_true (ct == 482)
				sliderTime = 25f
				ct = VertDistCount threshold 			 
				format "Number moved verts %\n" ct
				Assert_true (ct == 229)
			)			
		)
	),	

	Function MAXX_78825_Test = 
	(
		testName = "Volume_Select_Animated_With_Push"
		format "%\n" testName
		filePath = getFileNamePath(getSourceFileName())
		fileName = filePath + "/TestData/" + testName + ".max"
		isLoaded = loadMaxFile fileName quiet:true useFileUnits:true
		Assert_true isLoaded message: ("Error " + testName + " (load)")
		if (isLoaded) do
		(
			select $Sphere001
			
			-- Check bounds
			ValidateObjectBoundsAtTime $ 0f #([-140,-100,-100], [60,100,100]) (testName + " Bounds error1")

			ValidateObjectBoundsAtTime $ 10f #([-140,-100,-100], [70,100,100]) (testName + " Bounds error2")
		)
	),	

	Tests = #(
		Mesh_Test, Texture_Test, Vertex_Color_Test,
		Gizmo_Test, Smooth_Group_Test, MatID_Test,
		Animatable_Test,
		StackSelection_Test,
		MAXX_78825_Test
	)
)

if (run_test_fixture == undefined) then 
( 
	format "-- No fixture!\n" 
	testList = VolumeSelectUnitTests()
	for test in testList.Tests do
	(
		testList.setup()
		test()
		testList.teardown()
	)
) 
else 
( 
	format "-- Fixture found!\n" 
	run_test_fixture VolumeSelectUnitTests script:(getThisScriptFilename())
) 
